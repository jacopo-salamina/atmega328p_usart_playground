#include "my_timer.h"

#include <avr/io.h>
#include <Arduino.h>
#include <util/atomic.h>


/**
 * Task which will be queued after a specific timeout.
 * 
 * func is also used as a "flag": if it's set to NULL, that means no task was
 * scheduled (no function can have NULL as address), and thus no timeout is
 * pending.
 * 
 * Keep in mind that, since the output compare match interrupt is enabled, the
 * flag OCF1A on TIFR1 no longer serves this purpose, as it's automatically
 * cleared when running the associated ISR. Thus, we need to manually track the
 * status of the timeout.
 */
static task_t _scheduled_task =
{
  .func = NULL
};

ISR(TIMER1_COMPA_vect)
{
  /*
   * Queue the scheduled task.
   * 
   * Keep in mind that we're running inside an ISR, and by default nested ISRs
   * are not allowed, which means we don't need an atomic block for reading
   * _scheduled_task.
   */
  my_task__queue_new(_scheduled_task);
  _scheduled_task.func = NULL;
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

void my_timer__set_timeout(uint16_t delay_in_ms, task_t task)
{
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
    exit(1);
  }
  // If the supplied task is not valid, just quit.
  else if (task.func == NULL)
  {
    exit(1);
  }
  /*
   * If we already scheduled a task, just quit.
   * Otherwise, save the new task, so we can later queue it as soon as the
   * output compare match occurs.
   * 
   * Since structs cannot be atomically saved or loaded, we have to do this
   * inside an atomic block.
   */
  ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
  {
    if (_scheduled_task.func != NULL)
    {
      exit(1);
    }
    _scheduled_task = task;
  }
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
   * it into OCR1A (in order to trigger the output compare match). We don't care
   * about a potential overflow, as the counter would overflow as well and reach
   * OCR1A after exactly delay_in_timer_clock_cycles cycles.
   */
  OCR1A = TCNT1 + delay_in_timer_clock_cycles;
  /*
   * Enable the output compare match interrupt (so we can queue a new task as
   * soon as the output compare match occurs).
   */
  bitSet(TIMSK1, OCIE1A);
}

bool my_timer__is_timeout_pending()
{
  bool pending;
  /*
   * Since memory addresses are 16 bit wide, reading them atomically is not
   * possible; thus, we need an atomic block.
   */
  ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
  {
    pending = _scheduled_task.func != NULL;
  }
  return pending;
}