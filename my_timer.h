#pragma once

#include <stdbool.h>
#include <stdint.h>
#include "my_task.h"
#include "return_status.h"


#ifdef __cplusplus
extern "C"
{
#endif
/**
 * Configure timer 1.
 */
void my_timer__init();

/**
 * Schedule a specific callback to be executed after the specified amount of
 * milliseconds.
 */
return_status my_timer__set_timeout(uint16_t, my_task__func_t);

/**
 * Checks whether a timeout is still pending or not.
 * 
 * Note: this method lacks any atomicity safeguards, and is expected to be run
 * within an outer atomic block. This is intentional, as the main loop needs to
 * atomically check the state of the task manager, the USART module, the timer
 * and the ADC module.
 */
bool my_timer__is_timeout_pending();
#ifdef __cplusplus
}
#endif