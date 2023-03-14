#include "my_adc.h"

#include <avr/io.h>
#include <Arduino.h>
#include <util/atomic.h>


/*
 * Callback which will be invoked with the result of an ADC conversion. If
 * no conversion is currently running, its value is NULL.
 * 
 * This is also used as a "flag" which tells us whether a conversion is running.
 * In theory, we could rely on the flag ADCS on ADCSRA; however, when this flag
 * is cleared, the callback might not have been scheduled yet, and thus the ADC
 * conversion might not be over from a software perspective.
 */
static volatile my_task__func_t _conversion_complete_func = NULL;

ISR(ADC_vect)
{
  my_task__queue_new(
    (my_task__task_t){
      .func = _conversion_complete_func, .arg._ushort = ADC
    }
  );
  _conversion_complete_func = NULL;
}

void my_adc__init()
{
  /*
   * Reference voltage AV_CC, conversion result is right-adjusted, reading from
   * ADC0.
   */
  ADMUX = bit(REFS0);
  /* ADC enabled, no conversion started (yet), conversion complete interrupt
   * enabled, ADC clock divider 128.
   */
  ADCSRA = bit(ADEN) | bit(ADIE) | bit(ADPS2) | bit(ADPS1) | bit(ADPS0);
  /*
   * Digital input buffers disabled (I won't need them, might as well save on
   * power).
   */
  DIDR0 =
    bit(ADC5D) | bit(ADC4D) | bit(ADC3D) | bit(ADC2D) | bit(ADC1D) | bit(ADC0D);
}

return_status my_adc__start_conversion(my_task__func_t func)
{
  return_status status = return_status__ok;
  // A NULL function pointer is not valid.
  if (NULL == func)
  {
    status = return_status__my_adc__bad_parameter;
  }
  if (return_status__ok == status)
  {
    /*
      * If no callback was saved previously, that means there's no active
      * conversion: save our callback.
      * 
      * Function pointers are 16 bit wide and cannot be read atomically, thus we
      * need an atomic block for performing the aforementioned check.
      */
    ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
    {
      if (NULL != _conversion_complete_func)
      {
        status = return_status__my_adc__other;
      }
      else
      {
        _conversion_complete_func = func;
      }
    }
  }
  if (return_status__ok == status)
  {
    // Start the ADC conversion.
    bitSet(ADCSRA, ADSC);
  }
  return status;
}

bool my_adc__is_conversion_active()
{
  /*
   * Keep in mind that we're running inside an outer atomic block, so we can
   * read a function pointer without any problems.
   */
  return NULL != _conversion_complete_func;
}