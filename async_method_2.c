#include "async_method_2.h"

#include <stddef.h>
#include <stdint.h>
#include "my_adc.h"
#include "my_timer.h"
#include "my_usart.h"


#define ADC_STATS__BITS_AFTER_COMMA_FOR_AVG 2
#define ADC_STATS__MAX_VALUES (1 << ADC_STATS__BITS_AFTER_COMMA_FOR_AVG)

static struct
{
  uint16_t min, max, sum;
  uint8_t count;
}
_adc_stats =
{
  .count = 0
};

static char _byte_to_hex(unsigned char b)
{
  return b < 10 ? '0' + b : 'A' - 10 + b;
}

static void _print_hex_3_digits(char * dest, uint16_t n)
{
  dest[0] = _byte_to_hex((n >> 8) & 0xf);
  dest[1] = _byte_to_hex((n >> 4) & 0xf);
  dest[2] = _byte_to_hex(n & 0xf);
}

static return_status _async_method_2__adc_callback(my_task__arg_t arg)
{
  return_status status = return_status__ok;
  if (0 == _adc_stats.count)
  {
    _adc_stats.min = arg._ushort;
    _adc_stats.max = arg._ushort;
    _adc_stats.sum = arg._ushort;
    _adc_stats.count = 1;
  }
  else
  {
    if (arg._ushort < _adc_stats.min)
    {
      _adc_stats.min = arg._ushort;
    }
    if (arg._ushort > _adc_stats.max)
    {
      _adc_stats.max = arg._ushort;
    }
    _adc_stats.sum += arg._ushort;
    _adc_stats.count++;
    if (ADC_STATS__MAX_VALUES == _adc_stats.count)
    {
      _adc_stats.count = 0;
      status = MY_USART__WRITE_CONST_F("Avg. 0x");
      if (return_status__ok == status)
      {
        char hex_buffer[5];
        _print_hex_3_digits(
          hex_buffer, _adc_stats.sum >> ADC_STATS__BITS_AFTER_COMMA_FOR_AVG
        );
        hex_buffer[3] = '.';
        hex_buffer[4] = _byte_to_hex(
          (_adc_stats.sum & (ADC_STATS__MAX_VALUES - 1))
          << (4 - ADC_STATS__BITS_AFTER_COMMA_FOR_AVG)
        );
        status = my_usart__write_from_sram(hex_buffer, 5);
      }
      if (return_status__ok == status)
      {
        status = MY_USART__WRITE_CONST_F(" - Range 0x");
      }
      if (return_status__ok == status)
      {
        char hex_buffer[4];
        _print_hex_3_digits(hex_buffer, _adc_stats.max - _adc_stats.min);
        hex_buffer[3] = '\n';
        status = my_usart__write_from_sram(hex_buffer, 4);
      }
    }
  }
  return status;
}

return_status async_method_2__start(my_task__arg_t _bogus_arg)
{
  return_status status = my_timer__set_timeout(25, async_method_2__start);
  if (return_status__ok == status)
  {
    status = my_adc__start_conversion(_async_method_2__adc_callback);
  }
  return status;
}