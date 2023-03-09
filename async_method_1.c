#include "async_method_1.h"

#include <stdlib.h>
#include "my_task.h"
#include "my_timer.h"
#include "my_usart.h"


static void _async_method_1__task_2(void* _bogus_ptr)
{
  MY_USART__WRITE_CONST_F("EOF\nwe're done\n");
}

static void _async_method_1__task_1(void* ptr)
{
  async_method_1__method* method_ptr = (async_method_1__method*) ptr;
  /**
   * If the initial number of idle seconds hasn't passed yet, decrease the
   * number, then reschedule this task one second later.
   * Otherwise, we can schedule task 2.
   */
  if (method_ptr->data.seconds_left_to_wait_during_task_1 != 0)
  {
    method_ptr->data.seconds_left_to_wait_during_task_1--;
    my_timer__set_timeout(
      1000, (task_t) {.func = _async_method_1__task_1, .args = ptr}
    );
  }
  else
  {
    MY_USART__WRITE_CONST_F("almost there\n\n");
    my_timer__set_timeout(
      2000, (task_t) {.func = _async_method_1__task_2, .args = NULL}
    );
  }
}

async_method_1__method async_method_1__create()
{
  return (async_method_1__method)
  {
    .data =
    {
      .seconds_left_to_wait_during_task_1 = 10
    }
  };
}

void async_method_1__start(async_method_1__method* method_ptr)
{
  MY_USART__WRITE_CONST_F("wait for it\n\n");
  _async_method_1__task_1(method_ptr);
}