#include "my_usart.h"

#include <stdint.h>
#include <util/atomic.h>
#include <Arduino.h>


#define RING_BUFFER__MAX_SIZE 64
/**
 * Ring buffer used for storing the serial port's output data.
 */
static struct
{
  volatile char data[RING_BUFFER__MAX_SIZE];
  volatile uint8_t head, size;
}
_ring_buffer =
{
  .head = 0,
  .size = 0
};

/**
 * Quick'n'dirty replacement for memcpy, except:
 * - both src and dest are char arrays;
 * - dest is also volatile;
 * - up to 255 bytes may be copied (for convenience reasons only).
 */
static void _memcpy_volatile(
  volatile char * dest, const char * src, uint8_t size
)
{
  for (uint8_t i = 0; i < size; i++)
  {
    dest[i] = src[i];
  }
}

/**
 * Identical to _memcpy_volatile(), except src points to program memory.
 */
static void _memcpy_volatile_from_pgm(
  volatile char * dest, PGM_P src, uint8_t size
)
{
  for (uint8_t i = 0; i < size; i++)
  {
    dest[i] = pgm_read_byte(&src[i]);
  }
}

static return_status _my_usart__write_common_check_size_and_compute_tail(
  uint8_t size, uint8_t * tail_ptr
)
{
  return_status status = return_status__ok;
  uint8_t ring_buffer_size;
  uint8_t ring_buffer_head;
  /*
   * Since the data register empty ISR may modify the ring buffer's internal
   * pointers (head and size) anytime, we need to read both atomically.
   */
  ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
  {
    ring_buffer_size = _ring_buffer.size;
    ring_buffer_head = _ring_buffer.head;
  }
  uint8_t ring_buffer_free_space = RING_BUFFER__MAX_SIZE - ring_buffer_size;
  /*
   * If there's not enough space to accomodate data (according to the ring
   * buffer's internal state we read inside the atomic block), return an error.
   * 
   * Note that the ring buffer's internal state may be different now, since the
   * data register empty ISR may have been executed between the atomic block and
   * this branch. However, the ISR only consumes data from the buffer, so the
   * available space could have been increased, not shrinked.
   * 
   * It follows that, if there was enough space in the ring buffer when leaving
   * the atomic block, there's still enough space right now, and there will be
   * enough space afterwards.
   * 
   * Instead, if there was not enough space when leaving the atomic block, we
   * don't know whether there is enough space now or not. We chose to simply
   * return an error in this scenario, for simplicity's sake.
   */
  if (ring_buffer_free_space < size)
  {
    status = return_status__my_usart__overflow;
  }
  if (return_status__ok == status)
  {
    /*
     * We need to compute the ring buffer's tail now, once again according to
     * the buffer's state we read in the atomic block above.
     * 
     * It's true that the data register empty ISR may have modified the buffer's
     * state between the atomic block and this line of code. However, not only
     * did the ISR *not* shrink the available space, it also didn't touch the
     * buffer's tail (because, again, the ISR only consumes data).
     * 
     * It follows that the buffer's state we read above is sufficient for
     * computing its tail.
     */
    uint16_t ring_buffer_tail_not_wrapped = ring_buffer_head + ring_buffer_size;
    /*
     * We also need to keep in mind that the buffer's available data may wrap
     * around the internal array's end, and thus the tail may be behind the
     * head. This means that computing (head + size) is not enough, we also need
     * to wrap that value around the buffer's maximum size.
     */
    *tail_ptr =
      ring_buffer_tail_not_wrapped < RING_BUFFER__MAX_SIZE
      ? ring_buffer_tail_not_wrapped
      : ring_buffer_tail_not_wrapped - RING_BUFFER__MAX_SIZE;
  }
  return status;
}

static void _my_usart__write_common_update_ring_buffer_status(uint8_t size)
{
  /*
   * After copying data, we can finally increase the ring buffer's size, once
   * again using an atomic block.
   */
  ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
  {
    uint8_t ring_buffer_previous_size = _ring_buffer.size;
    /*
     * Additionally, if there were no bytes to transmit before (size was zero
     * before updating it), this means the data register empty interrupt has
     * been disabled, and we need to enable it back. We also need to clear the
     * flag TXC0 on UCSR0A, so we can later poll that flag in order to know when
     * will the transmission end.
     */
    if (ring_buffer_previous_size == 0)
    {
      bitSet(UCSR0B, UDRIE0);
      bitSet(UCSR0A, TXC0);
    }
    _ring_buffer.size = ring_buffer_previous_size + size;
  }
}

ISR(USART_UDRE_vect)
{
  /*
   * Read the ring buffer's byte pointed to by its head, and copy it to the
   * USART module's buffer.
   * 
   * Keep in mind that we're running inside an ISR, and by default nested ISRs
   * are not allowed, which means we don't need an atomic block.
   */
  uint8_t ring_buffer_previous_head = _ring_buffer.head;
  UDR0 = _ring_buffer.data[ring_buffer_previous_head];
  /*
   * Advance the ring buffer's head, and move it back to 0 if it would point
   * beyond data's end.
   */
  _ring_buffer.head =
    ring_buffer_previous_head == RING_BUFFER__MAX_SIZE - 1
    ? 0
    : ring_buffer_previous_head + 1;
  /*
   * Decrement the ring buffer's size by 1.
   * 
   * Additionally, if size becomes 0, we need to disable the data register
   * empty's interrupt. Otherwise, since we're not going to push new bytes, this
   * ISR would be triggered over and over again (according to section 19.6.3).
   */
  uint8_t ring_buffer_new_size = --(_ring_buffer.size);
  if (ring_buffer_new_size == 0)
  {
    bitClear(UCSR0B, UDRIE0);
  }
}

return_status my_usart__init(uint16_t baud_rate)
{
  return_status status = return_status__ok;
  // Keep in mind that we're going to set the baud rate divider to 16.
  uint16_t UBRR0_value = F_CPU / (16L * baud_rate) - 1;
  /*
   * According to section 19.10.5, UBRR0H's 4 most significant bits must be
   * zero. If the value of baud_rate would violate the aforementioned
   * condition, we just stop.
   */
  if (UBRR0_value & 0xf000)
  {
    status = return_status__my_usart__bad_parameter;
  }
  if (return_status__ok == status)
  {
    /*
     * We need interrupts enabled after configuring USART; that's why we're
     * annotating the atomic block with ATOMIC_FORCEON.
     */
    ATOMIC_BLOCK(ATOMIC_FORCEON)
    {
      UBRR0H = UBRR0_value >> 8;
      UBRR0L = UBRR0_value & 0xff;
      /*
       * No interrupts enabled (for now), only transmitter enabled, 8 bit
       * characters (continued below).
       */
      UCSR0B = bit(TXEN0);
      // Baud rate divider set to 16.
      bitClear(UCSR0A, U2X0);
      /*
       * Asynchronous USART, no parity bit, one stop bit, 8 bit characters.
       */
      UCSR0C = bit(UCSZ01) | bit(UCSZ00);
      //UCSR0C = bit(UPM01) | bit(USBS0) | bit(UCSZ01) | bit(UCSZ00);
    }
  }
  return status;
}

return_status my_usart__write_from_sram(const char * data, uint8_t size)
{
  // Edge case where there's no data to write; we don't need to do anything.
  if (size == 0)
  {
    return return_status__ok;
  }
  uint8_t ring_buffer_tail;
  return_status status = _my_usart__write_common_check_size_and_compute_tail(
    size, &ring_buffer_tail
  );
  if (return_status__ok == status)
  {
    /*
     * It may not be possible to copy data in a single pass, because the
     * available bytes between the current tail and the end of the array may
     * less than size.
     */
    if (ring_buffer_tail <= RING_BUFFER__MAX_SIZE - size)
    {
      _memcpy_volatile(&_ring_buffer.data[ring_buffer_tail], data, size);    
    }
    else
    {
      uint8_t first_batch_size = RING_BUFFER__MAX_SIZE - ring_buffer_tail;
      _memcpy_volatile(
        &_ring_buffer.data[ring_buffer_tail], data, first_batch_size
      );
      _memcpy_volatile(
        _ring_buffer.data, &data[first_batch_size], size - first_batch_size
      );
    }
    _my_usart__write_common_update_ring_buffer_status(size);
  }
  return status;
}

return_status my_usart__write_from_pgm(PGM_P data, uint8_t size)
{
  return_status status = return_status__ok;
  // Edge case where there's no data to write; we don't need to do anything.
  if (size == 0)
  {
    return status;
  }
  uint8_t ring_buffer_tail;
  status = _my_usart__write_common_check_size_and_compute_tail(
    size, &ring_buffer_tail
  );
  if (return_status__ok == status)
  {
    /*
     * It may not be possible to copy data in a single pass, because the
     * available bytes between the current tail and the end of the array may
     * less than size.
     */
    if (ring_buffer_tail <= RING_BUFFER__MAX_SIZE - size)
    {
      _memcpy_volatile_from_pgm(
        &_ring_buffer.data[ring_buffer_tail], data, size
      );
    }
    else
    {
      uint8_t first_batch_size = RING_BUFFER__MAX_SIZE - ring_buffer_tail;
      _memcpy_volatile_from_pgm(
        &_ring_buffer.data[ring_buffer_tail], data, first_batch_size
      );
      _memcpy_volatile_from_pgm(
        _ring_buffer.data, &data[first_batch_size], size - first_batch_size
      );
    }
    _my_usart__write_common_update_ring_buffer_status(size);
  }
  return status;
}

bool my_usart__is_transmission_active()
{
  return 0 != _ring_buffer.size;
}