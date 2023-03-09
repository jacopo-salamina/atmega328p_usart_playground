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
void my_task__queue_new(task_t);

bool my_task__try_to_run_next();
#ifdef __cplusplus
}
#endif