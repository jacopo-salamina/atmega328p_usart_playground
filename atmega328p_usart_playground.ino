/**
 * TODO Make sure type casting is not needed
 * TODO Should I write (n == 0) or (!n) ?
 * TODO Review struct initialization (empty vs default initialization, BSS vs
 *  init functions)
 * TODO Review volatile usage (did I abuse it?)
 * TODO How should interrupts check whether my_task__queue_new doesn't return an
 *  error?
 */

#include <util/atomic.h>
#include "async_method_2.h"
#include "my_adc.h"
#include "my_task.h"
#include "my_timer.h"
#include "my_usart.h"
#include "return_status.h"


#define ASSERT_OK(expr) \
do \
{ \
  if (return_status__ok != expr) \
  { \
    return 1; \
  } \
} \
while (0)

int main()
{
  my_adc__init();
  my_timer__init();
  ASSERT_OK(my_usart__init(9600));
  ASSERT_OK(async_method_2__start());
  bool loop_running;
  /*
   * Keep checking whether any of the following conditions hold true:
   * - there's a new task we can immediately run;
   * - there's a task scheduled to be run in the future;
   * - the USART module is still transmitting.
   * If all these conditions are false, that means there's nothing left to do
   * anymore: just quit.
   * 
   * Since this global state may be altered anytime by an ISR, we need to
   * inspect all the conditions inside the same atomic block.
   * 
   * Additionally, if a new task was found, execute it outside of the
   * aforementioned atomic block, in order not to block any ISR.
   */
  do
  {
    my_task__task_t next_task_found;
    ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
    {
      next_task_found = my_task__try_to_read_next();
      loop_running =
        NULL != next_task_found.func
        || my_timer__is_timeout_pending()
        || my_usart__is_transmission_active();
    }
    if (NULL != next_task_found.func)
    {
      ASSERT_OK(next_task_found.func(next_task_found.arg));
    }
  }
  while (loop_running);
}