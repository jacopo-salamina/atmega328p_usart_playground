#include "async_method_1.h"

#include <stdlib.h>
#include "my_task.h"
#include "my_timer.h"
#include "my_usart.h"


static uint8_t _seconds_left_to_wait_during_task_1 = 10;

static return_status _async_method_1__task_2(void* _bogus_ptr)
{
  return MY_USART__WRITE_CONST_F("EOF\nwe're done\n");
}

static return_status _async_method_1__task_1(void* _bogus_ptr)
{
  return_status status = return_status__ok;
  /**
   * If the initial number of idle seconds hasn't passed yet, decrease the
   * number, then reschedule this task one second later.
   * Otherwise, we can schedule task 2.
   */
  if (_seconds_left_to_wait_during_task_1 != 0)
  {
    _seconds_left_to_wait_during_task_1--;
    status = my_timer__set_timeout(
      1000, (task_t) {.func = _async_method_1__task_1, .args = NULL}
    );
  }
  else
  {
    MY_USART__WRITE_CONST_F("almost there\n\n");
    status = my_timer__set_timeout(
      2000, (task_t) {.func = _async_method_1__task_2, .args = NULL}
    );
  }
  return status;
}

return_status async_method_1__start()
{
  return_status status = MY_USART__WRITE_CONST_F("wait for it\n\n");
  if (return_status__ok == status)
  {
    status = _async_method_1__task_1(NULL);
  }
  return status;
}