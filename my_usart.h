#pragma once

#include <stdint.h>
#include <avr/pgmspace.h>


typedef enum
{
  my_usart__return_status_ok,
  my_usart__return_status_overflow,
  my_usart__return_status_bad_parameter,
  my_usart__return_status_other
}
my_usart__return_status;

#ifdef __cplusplus
extern "C"
{
#endif
my_usart__return_status my_usart__init(uint16_t);

my_usart__return_status my_usart__write(const char*, uint8_t);

my_usart__return_status my_usart__write_from_pgm(PGM_P, uint8_t);

void my_usart__flush();
#ifdef __cplusplus
}
#endif