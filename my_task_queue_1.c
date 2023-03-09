#include "my_task_queue_1.h"

#include <stdlib.h>
#include "my_task.h"
#include "my_timer.h"
#include "my_usart.h"


static void _my_task_queue_1__task_2(void*);

static void _my_task_queue_1__task_1(void* ptr)
{
  my_task_queue_1__queue* queue_ptr = (my_task_queue_1__queue*) ptr;
  if (queue_ptr->data.seconds_left_to_wait_during_task_1 != 0)
  {
    queue_ptr->data.seconds_left_to_wait_during_task_1--;
    my_timer__set_timeout(
      1000, (task_t) {.func = _my_task_queue_1__task_1, .args = ptr}
    );
  }
  else
  {
    MY_USART__WRITE_CONST_F("almost there\n\n");
    my_timer__set_timeout(
      2000, (task_t) {.func = _my_task_queue_1__task_2, .args = NULL}
    );
  }
}

static void _my_task_queue_1__task_2(void* _bogus_ptr)
{
  MY_USART__WRITE_CONST_F("EOF\nwe're done\n");
}

my_task_queue_1__queue my_task_queue_1__create()
{
  return (my_task_queue_1__queue)
  {
    .data =
    {
      .seconds_left_to_wait_during_task_1 = 10
    }
  };
}

void my_task_queue_1__start(my_task_queue_1__queue* queue_ptr)
{
  MY_USART__WRITE_CONST_F("wait for it\n\n");
  _my_task_queue_1__task_1(queue_ptr);
}