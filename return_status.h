#pragma once

#include <stdint.h>


typedef enum
{
  return_status__ok,
  // USART-related states
  return_status__my_usart__bad_parameter,
  return_status__my_usart__overflow,
  return_status__my_usart__other,
  // Timer-related states
  return_status__my_timer__bad_parameter,
  return_status__my_timer__other,
  // Task-related states
  return_status__my_task__bad_parameter,
  return_status__my_task__overflow,
  return_status__my_task__other,
  // ADC-related states
  return_status__my_adc__bad_parameter,
  return_status__my_adc__other,
}
return_status_t;

typedef uint8_t return_status_byte_t;