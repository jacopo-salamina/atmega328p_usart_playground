#include "my_timer.h"

#include <avr/io.h>
#include <Arduino.h>
#include <util/atomic.h>


/**
 * Callback which will be turned into an empty argument task after a specific
 * timeout.
 * 
 * This is also used as a "flag": if it's set to NULL, that means no callback
 * was scheduled (no function can have NULL as address), and thus no timeout is
 * pending.
 * 
 * Keep in mind that, since the output compare match interrupt is enabled, the
 * flag OCF1A on TIFR1 no longer serves this purpose, as it's automatically
 * cleared when running the associated ISR. Thus, we need to manually track the
 * status of the timeout.
 */
static my_task__func_t _scheduled_func = NULL;

ISR(TIMER1_COMPA_vect)
{
  /*
   * Queue a task which runs _scheduled_func with the empty argument.
   * 
   * Keep in mind that we're running inside an ISR, and by default nested ISRs
   * are not allowed, which means we don't need an atomic block for reading
   * _scheduled_func.
   */
  my_task__queue_new(
    (my_task__task_t){.func = _scheduled_func, .arg = MY_TASK__EMPTY_ARG}
  );
  _scheduled_func = NULL;
  /*
   * Disable the output compare match interrupt; otherwise, this ISR would run
   * twice, without a valid task to queue.
   */
  bitClear(TIMSK1, OCIE1A);
}

void my_timer__init()
{
  /*
   * No output compare units activated, timer operating in normal mode
   * (continued).
   */
  TCCR1A = 0;
  // Timer operating in normal mode, internal clock (prescaler set to 1024).
  TCCR1B = bit(CS12) | bit(CS10);
  // No interrupts enabled (for now).
  TIMSK1 = 0;
  // Reset the counter.
  TCNT1 = 0;
  /*
   * Output compare register A temporarily set to 0 (until somebody invokes
   * my_timer_set_delay()).
   */
  OCR1A = 0;
}

return_status_byte_t my_timer__set_timeout(
  uint16_t delay_in_ms, my_task__func_t func
)
{
  return_status_byte_t status = return_status__ok;
  /*
   * The timer's internal counter wraps back to 0 approximately every 4194.3 ms.
   * If we tried to generate a 4195 ms (or longer) delay, we would need to
   * increase OCR1A with a value greater than 0x10000, but adding 0x10000 to a
   * number doesn't affect its last 16 bits, which means OCR1A would be
   * increased by a few timer clock cycles, and the counter would match that
   * value after less than a millisecond, which is not correct.
   */
  if (delay_in_ms > 4194)
  {
    status = return_status__my_timer__bad_parameter;
  }
  else if (NULL == func)
  {
    status = return_status__my_timer__bad_parameter;
  }
  if (return_status__ok == status)
  {
    /*
     * If we didn't schedule a task already, save the new callback, so we can
     * later queue a task as soon as the output compare match occurs.
     * 
     * Since function pointers cannot be atomically saved or loaded, we have to
     * do this inside an atomic block.
     */
    ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
    {
      if (NULL != _scheduled_func)
      {
        status = return_status__my_timer__other;
      }
      else
      {
        _scheduled_func = func;
      }
    }
  }
  if (return_status__ok == status)
  {
    /*
     * Convert the delay into an equivalent number of timer clocks.
     * 
     * This results in a small rounding error, which builds up over time and
     * messes up longer delays (e.g. a one minute delay gets smaller by 0.1
     * seconds). Since we don't need to generate very long delays, and also for
     * simplicity's sake, we're ignoring this error.
     */
    uint16_t delay_in_timer_clock_cycles = delay_in_ms * F_CPU / 1024 / 1000;
    /*
     * Compute the future value of TCNT1 after delay_ms milliseconds, and store
     * it into OCR1A (in order to trigger the output compare match). We don't
     * care about a potential overflow, as the counter would overflow as well
     * and reach OCR1A after exactly delay_in_timer_clock_cycles cycles.
     */
    OCR1A = TCNT1 + delay_in_timer_clock_cycles;
    /*
     * Enable the output compare match interrupt (so we can queue a new task as
     * soon as the output compare match occurs).
     */
    bitSet(TIMSK1, OCIE1A);
  }
  return status;
}

bool my_timer__is_timeout_pending()
{
  // Keep in mind, this method is expected to run inside an outer atomic block.
  return NULL != _scheduled_func;
}