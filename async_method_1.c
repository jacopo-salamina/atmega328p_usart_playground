#include "async_method_1.h"

#include <stdlib.h>
#include "my_task.h"
#include "my_timer.h"
#include "my_usart.h"


static uint8_t _seconds_before_scheduling_task_2 = 10;

static return_status_byte_t _async_method_1__task_2(my_task__arg_t _bogus_arg)
{
  return MY_USART__WRITE_CONST_F("EOF\nwe're done\n");
}

static return_status_byte_t _async_method_1__task_1(my_task__arg_t _bogus_arg)
{
  return_status_byte_t status = return_status__ok;
  /**
   * If the initial number of idle seconds hasn't passed yet, decrease the
   * number, then reschedule this task one second later.
   * Otherwise, we can schedule task 2.
   */
  if (0 != _seconds_before_scheduling_task_2)
  {
    _seconds_before_scheduling_task_2--;
    status = my_timer__set_timeout(1000, _async_method_1__task_1);
  }
  else
  {
    MY_USART__WRITE_CONST_F("almost there\n\n");
    status = my_timer__set_timeout(2000, _async_method_1__task_2);
  }
  return status;
}

return_status_byte_t async_method_1__start()
{
  return_status_byte_t status = MY_USART__WRITE_CONST_F("wait for it\n\n");
  if (return_status__ok == status)
  {
    status = _async_method_1__task_1(MY_TASK__EMPTY_ARG);
  }
  return status;
}