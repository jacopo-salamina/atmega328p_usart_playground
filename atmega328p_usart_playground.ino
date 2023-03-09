/**
 * TODO Make sure type casting is not needed
 * TODO Review struct initialization, as well as static and volatile attributes
 *  on members
 * TODO Should I write (n == 0) or (!n) ?
 */

#include "my_timer.h"
#include "my_usart.h"


#define MY_USART__WRITE_CONST(str) my_usart__write(str, (sizeof str) - 1)
#define MY_USART__WRITE_CONST_F(str) \
my_usart__write_from_pgm(PSTR(str), (sizeof str) - 1)

int main()
{
  my_timer__init();
  if (my_usart__return_status_ok != my_usart__init(9600))
  {
    return 1;
  }
  if (my_usart__return_status_ok != MY_USART__WRITE_CONST_F("wait for it\n\n"))
  {
    return 1;
  }
  for (uint8_t i = 0; i < 10; i++)
  {
    if (my_timer__return_status_ok != my_timer__wait(1000))
    {
      return 1;
    }
  }
  if (my_usart__return_status_ok != MY_USART__WRITE_CONST_F("almost there\n\n"))
  {
    return 1;
  }
  if (my_timer__return_status_ok != my_timer__wait(2000))
  {
    return 1;
  }
  if (
    my_usart__return_status_ok != MY_USART__WRITE_CONST_F("EOF\nwe're done\n")
  )
  {
    return 1;
  }
  my_usart__flush();
}