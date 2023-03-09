#pragma once

#include <stdbool.h>


typedef struct
{
  void (* func)(void*);
  void* args;
}
task_t;

#ifdef __cplusplus
extern "C"
{
#endif
/**
 * Queue a new task instance (if there's enough available space).
 */
void my_task__queue_new(task_t);

/**
 * Attempt to retrieve the next available task. If no task is found, instead
 * of waiting for a new one, return an invalid task (where func is NULL).
 * 
 * Note: this method lacks any atomicity safeguards, and is expected to be run
 * within an outer atomic block. This is intentional, as the main loop needs to
 * atomically check the state of this task manager, the USART module and the
 * timer.
 */
task_t my_task__try_to_read_next();
#ifdef __cplusplus
}
#endif