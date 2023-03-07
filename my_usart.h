#pragma once

#include <stdint.h>
#include <avr/pgmspace.h>


#ifdef __cplusplus
extern "C"
{
#endif
void my_usart_init(uint16_t);

void my_usart_write(const char*, uint8_t);

void my_usart_write_from_pgm(PGM_P, uint8_t);

void my_usart_flush();
#ifdef __cplusplus
}
#endif