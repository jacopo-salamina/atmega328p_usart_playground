#include "my_timer.h"

#include <avr/io.h>
#include <Arduino.h>


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

void my_timer__wait(uint16_t delay_in_ms)
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
   * Clear OCF1A on TIFR1, as we're going to poll that flag later in order to
   * know when the output compare match occurred.
   */
  bitSet(TIFR1, OCF1A);
  /**
   * Poll the flag OCF1A on TIFR1, until the flag becomes 1, which means that
   * the output compare match with OCR1A occurred, and thus the delay specified
   * in my_timer_set_delay() is over.
   */
  while (!(TIFR1 & bit(OCF1A)));
}