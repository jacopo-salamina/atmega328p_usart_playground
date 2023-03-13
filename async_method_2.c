#include "async_method_2.h"

#include "my_adc.h"
#include "my_task.h"
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
  return my_usart__write_from_sram(value_str, 6);
}

return_status async_method_2__start()
{
  return my_adc__start_conversion(_async_method_2__adc_callback);
}