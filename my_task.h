#pragma once

#include <stdbool.h>
#include <stdint.h>
#include "return_status.h"


typedef union
{
  uint8_t _ubyte;
  uint16_t _ushort;
  void * _void_ptr;
}
my_task__arg_t;

typedef struct
{
  return_status (* func)(my_task__arg_t);
  my_task__arg_t arg;
}
my_task__task_t;

#define MY_TASK__EMPTY_ARG ((my_task__arg_t){._void_ptr = NULL})

#ifdef __cplusplus
extern "C"
{
#endif
/**
 * Queue a new task instance (if there's enough available space).
 */
return_status my_task__queue_new(my_task__task_t);

/**
 * Attempt to retrieve the next available task. If no task is found, instead
 * of waiting for a new one, return an invalid task (where func is NULL).
 * 
 * Note: this method lacks any atomicity safeguards, and is expected to be run
 * within an outer atomic block. This is intentional, as the main loop needs to
 * atomically check the state of this task manager, the USART module and the
 * timer.
 */
my_task__task_t my_task__try_to_read_next();
#ifdef __cplusplus
}
#endif