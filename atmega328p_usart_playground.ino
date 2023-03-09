/**
 * TODO Make sure type casting is not needed
 * TODO Should I write (n == 0) or (!n) ?
 * TODO Review struct initialization (empty vs default initialization, BSS vs
 *  init functions)
 * TODO Review volatile usage (did I abuse it?)
 */

#include <util/atomic.h>
#include "async_method_1.h"
#include "my_task.h"
#include "my_timer.h"
#include "my_usart.h"


int main()
{
  my_timer__init();
  my_usart__init(9600);
  async_method_1__method method = async_method_1__create();
  async_method_1__start(&method);
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
    task_t next_task_found;
    ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
    {
      next_task_found = my_task__try_to_read_next();
      loop_running =
        next_task_found.func != NULL
        || my_timer__is_timeout_pending()
        || my_usart__is_transmission_active();
    }
    if (next_task_found.func != NULL)
    {
      next_task_found.func(next_task_found.args);
    }
  }
  while (loop_running);
}