#pragma once

#include "my_task.h"
#include "return_status.h"


#ifdef __cplusplus
extern "C"
{
#endif
/**
 * Initializes the ADC module.
 */
void my_adc__init();

/**
 * Starts an ADC conversion, which, when completed, will trigger the specified
 * callback with the conversion result.
 */
return_status_byte_t my_adc__start_conversion(my_task__func_t);

/**
 * Checks whether there's an ongoing ADC conversion.
 * 
 * Note: this method lacks any atomicity safeguards, and is expected to be run
 * within an outer atomic block. This is intentional, as the main loop needs to
 * atomically check the state of the task manager, the USART module, the timer
 * and the ADC module.
 */
bool my_adc__is_conversion_active();
#ifdef __cplusplus
}
#endif