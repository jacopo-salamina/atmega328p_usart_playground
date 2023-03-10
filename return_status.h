#pragma once


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
}
return_status;