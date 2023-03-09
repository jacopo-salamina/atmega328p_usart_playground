/**
 * TODO Make sure type casting is not needed
 * TODO Review struct initialization, as well as static and volatile attributes
 *  on members
 * TODO Should I write (n == 0) or (!n) ?
 * TODO Review struct initialization (empty vs default initialization)
 * TODO Review volatile usage (did I abuse it?)
 */

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
  while
  (
    my_task__try_to_run_next()
    || my_timer__is_timeout_pending()
    || my_usart__is_transmission_active()
  );
  bitClear(PORTB, PORTB5);
}