#include "my_usart.h"

#include <stdint.h>
#include <util/atomic.h>
#include <Arduino.h>


#define RING_BUFFER_MAX_SIZE 127
/**
 * Ring buffer used for storing the serial port's output data.
 */
static struct
{
  volatile uint8_t head, size;
  volatile char data[RING_BUFFER_MAX_SIZE];
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
static void _memcpy_volatile(volatile char* dest, const char* src, uint8_t size)
{
  for (uint8_t i = 0; i < size; i++)
  {
    dest[i] = src[i];
  }
}

ISR(USART_UDRE_vect)
{
  /*
   * Read the ring buffer's byte pointed to by its head, and copy it to the
   * USART module's buffer.
   */
  uint8_t ring_buffer_previous_head = _ring_buffer.head;
  UDR0 = _ring_buffer.data[ring_buffer_previous_head];
  /*
   * Advance the ring buffer's head, and move it back to 0 if it would point
   * beyond data's end.
   */
  _ring_buffer.head =
    ring_buffer_previous_head == RING_BUFFER_MAX_SIZE - 1
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

void my_usart_init(uint16_t baud_rate)
{
  // Keep in mind that we're going to set the baud rate divider to 16.
  uint16_t UBRR0_value = F_CPU / (16L * baud_rate) - 1;
  /*
   * According to section 19.10.5, UBRR0H's 4 most significant bits must be
   * zero. If the value of baud_rate would violate the aforementioned
   * condition, we just stop.
   */
  if (UBRR0_value & 0xf000)
  {
    exit(1);
  }
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

void my_usart_write(const char* data, uint8_t size)
{
  // SANITY CHECKS

  // Edge case where there's no data to write; we don't need to do anything.
  if (size == 0)
  {
    return;
  }
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
  uint8_t ring_buffer_free_space = RING_BUFFER_MAX_SIZE - ring_buffer_size;
  /*
   * If there's not enough space to accomodate data (according to the ring
   * buffer's internal state we read inside the atomic block), we just quit.
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
   * quit in this scenario, for simplicity's sake.
   */
  if (ring_buffer_free_space < size)
  {
    exit(1);
  }

  // ACTUAL BUFFER WRITES

  /*
   * We need to compute the ring buffer's tail now, once again according to the
   * buffer's state we read in the atomic block above.
   * 
   * It's true that the data register empty ISR may have modified the buffer's
   * state between the atomic block and this line of code. However, not only
   * did the ISR *not* shrink the available space, it also didn't touch the
   * buffer's tail (because, again, the ISR only consumes data).
   * 
   * It follows that the buffer's state we read above is sufficient for
   * computing its tail.
   * 
   * We also need to keep in mind that the buffer's available data may wrap
   * around the internal array's end, and thus the tail may be behind the head.
   * This means that computing (head + size) is not enough, we also need to wrap
   * that value around the buffer's maximum size.
   */
  uint16_t ring_buffer_tail_not_wrapped = ring_buffer_head + ring_buffer_size;
  uint8_t ring_buffer_tail =
    ring_buffer_tail_not_wrapped < RING_BUFFER_MAX_SIZE
    ? ring_buffer_tail_not_wrapped
    : ring_buffer_tail_not_wrapped - RING_BUFFER_MAX_SIZE;
  /*
   * It may not be possible to copy data in a single pass, because the available
   * bytes between the current tail and the end of the array may less than size.
   */
  if (ring_buffer_tail <= RING_BUFFER_MAX_SIZE - size)
  {
    _memcpy_volatile(&_ring_buffer.data[ring_buffer_tail], data, size);    
  }
  else
  {
    uint8_t first_batch_size = RING_BUFFER_MAX_SIZE - ring_buffer_tail;
    _memcpy_volatile(
      &_ring_buffer.data[ring_buffer_tail], data, first_batch_size
    );
    _memcpy_volatile(
      _ring_buffer.data, &data[first_batch_size], size - first_batch_size
    );
  }
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

/**
 * Polls the flag TXC0 on UCSR0A until it becomes 1 (which means the USART
 * module has no more bytes to transmit).
 */
void my_usart_flush()
{
  while (!(UCSR0A & bit(TXC0)));
}