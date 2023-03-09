#pragma once

#include <stdbool.h>
#include <stdint.h>
#include <avr/pgmspace.h>


#define MY_USART__WRITE_CONST(str) my_usart__write(str, (sizeof str) - 1)
#define MY_USART__WRITE_CONST_F(str) \
my_usart__write_from_pgm(PSTR(str), (sizeof str) - 1)

#ifdef __cplusplus
extern "C"
{
#endif
void my_usart__init(uint16_t);

void my_usart__write(const char*, uint8_t);

void my_usart__write_from_pgm(PGM_P, uint8_t);

bool my_usart__is_transmission_active();
#ifdef __cplusplus
}
#endif