#pragma once

#include <stdint.h>


typedef struct
{
  struct
  {
    uint8_t seconds_left_to_wait_during_task_1;
  }
  data;
}
my_task_queue_1__queue;

#ifdef __cplusplus
extern "C"
{
#endif
my_task_queue_1__queue my_task_queue_1__create();

void my_task_queue_1__start(my_task_queue_1__queue* queue);
#ifdef __cplusplus
}
#endif