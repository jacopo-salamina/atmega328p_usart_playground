#pragma once

#include <stdint.h>
#include <avr/pgmspace.h>


#ifdef __cplusplus
extern "C"
{
#endif
void my_usart__init(uint16_t);

void my_usart__write(const char*, uint8_t);

void my_usart__write_from_pgm(PGM_P, uint8_t);

void my_usart__flush();
#ifdef __cplusplus
}
#endif