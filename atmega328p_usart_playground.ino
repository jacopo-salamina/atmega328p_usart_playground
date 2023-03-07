/**
 * TODO Make sure type casting is not needed
 * TODO Review struct initialization, as well as static and volatile attributes
 *  on members
 * TODO Should I write (n == 0) or (!n) ?
 */

#include "my_timer.h"
#include "my_usart.h"


#define MY_USART_WRITE_CONST(str) my_usart_write(str, (sizeof str) - 1)
#define MY_USART_WRITE_CONST_F(str) \
my_usart_write_from_pgm(PSTR(str), (sizeof str) - 1)

int main()
{
  my_timer_init();
  my_usart_init(9600);
  MY_USART_WRITE_CONST_F("wait for it\n\n");
  for (uint8_t i = 0; i < 10; i++)
  {
    my_timer_set_delay(1000);
    my_timer_wait();
  }
  my_timer_set_delay(2000);
  MY_USART_WRITE_CONST_F("almost there\n\n");
  my_timer_wait();
  MY_USART_WRITE_CONST_F("EOF\nwe're done\n");
  my_usart_flush();
}