/**
 * TODO Make sure type casting is not needed
 * TODO Review struct initialization, as well as static and volatile attributes
 *  on members
 * TODO Should I write (n == 0) or (!n) ?
 */

#include "my_timer.h"
#include "my_usart.h"


#define TEST_STRING_1 "wait for it\n\n"
#define TEST_STRING_2 "almost there\n\n"
#define TEST_STRING_3 "EOF\nwe're done\n"

int main()
{
  my_timer_init();
  my_usart_init(9600);
  my_usart_write(TEST_STRING_1, (sizeof TEST_STRING_1) - 1);
  for (uint8_t i = 0; i < 60; i++)
  {
    my_timer_set_delay(1000);
    my_timer_wait();
  }
  my_timer_set_delay(2000);
  my_usart_write(TEST_STRING_2, (sizeof TEST_STRING_2) - 1);
  my_timer_wait();
  my_usart_write(TEST_STRING_3, (sizeof TEST_STRING_3) - 1);
  my_usart_flush();
}