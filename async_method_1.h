#pragma once

#include <stdint.h>


/**
 * Stores data shared among the different tasks of this async method.
 */
typedef struct
{
  struct
  {
    uint8_t seconds_left_to_wait_during_task_1;
  }
  data;
}
async_method_1__method;

#ifdef __cplusplus
extern "C"
{
#endif
/**
 * Create a new instance of this method.
 */
async_method_1__method async_method_1__create();

/**
 * Start running a specific instance of this method.
 */
void async_method_1__start(async_method_1__method* method);
#ifdef __cplusplus
}
#endif