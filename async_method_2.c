#include "async_method_2.h"

#include <stddef.h>
#include "my_adc.h"
#include "my_timer.h"
#include "my_usart.h"


static char byte_to_hex(unsigned char b)
{
  return b < 10 ? '0' + b : 'A' - 10 + b;
}

static return_status _async_method_2__adc_callback(my_task__arg_t arg)
{
  char value_str[] = {
    '0',
    'x',
    byte_to_hex(arg._ushort >> 8),
    byte_to_hex((arg._ushort >> 4) & 0xf),
    byte_to_hex(arg._ushort & 0xf),
    '\n'
  };
  return_status status = my_usart__write_from_sram(value_str, 6);
  if (return_status__ok == status)
  {
    status = my_timer__set_timeout(100, async_method_2__start);
  }
  return status;
}

return_status async_method_2__start(my_task__arg_t _bogus_arg)
{
  return my_adc__start_conversion(_async_method_2__adc_callback);
}