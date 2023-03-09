/**
 * TODO Make sure type casting is not needed
 * TODO Should I write (n == 0) or (!n) ?
 * TODO Review struct initialization (empty vs default initialization, BSS vs
 *  init functions)
 * TODO Review volatile usage (did I abuse it?)
 */

#include <util/atomic.h>
#include "my_task_queue_1.h"
#include "my_task.h"
#include "my_timer.h"
#include "my_usart.h"


int main()
{
  bitSet(DDRB, DDB5);
  bitSet(PORTB, PORTB5);
  my_timer__init();
  my_usart__init(9600);
  my_task_queue_1__queue queue = my_task_queue_1__create();
  my_task_queue_1__start(&queue);
  bool loop_running;
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
  bitClear(PORTB, PORTB5);
}