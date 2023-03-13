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
return_status my_adc__start_conversion(my_task__func_t);
#ifdef __cplusplus
}
#endif