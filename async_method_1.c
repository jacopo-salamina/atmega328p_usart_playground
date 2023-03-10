#include "async_method_1.h"

#include <stdlib.h>
#include "my_task.h"
#include "my_timer.h"
#include "my_usart.h"


static return_status _async_method_1__task_2(my_task__arg_t _bogus_arg)
{
  return MY_USART__WRITE_CONST_F("EOF\nwe're done\n");
}

static return_status _async_method_1__task_1(my_task__arg_t arg)
{
  return_status status = return_status__ok;
  /**
   * If the initial number of idle seconds hasn't passed yet, decrease the
   * number, then reschedule this task one second later.
   * Otherwise, we can schedule task 2.
   */
  if (arg._ubyte != 0)
  {
    status = my_timer__set_timeout(
      1000,
      (my_task__task_t) {
        .func = _async_method_1__task_1, .arg._ubyte = arg._ubyte - 1
      }
    );
  }
  else
  {
    MY_USART__WRITE_CONST_F("almost there\n\n");
    status = my_timer__set_timeout(
      2000,
      (my_task__task_t) {
        .func = _async_method_1__task_2, .arg = MY_TASK__EMPTY_ARG
      }
    );
  }
  return status;
}

return_status async_method_1__start()
{
  return_status status = MY_USART__WRITE_CONST_F("wait for it\n\n");
  if (return_status__ok == status)
  {
    status = _async_method_1__task_1((my_task__arg_t){._ubyte = 10});
  }
  return status;
}