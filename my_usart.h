#pragma once

#include <stdint.h>


#ifdef __cplusplus
extern "C"
{
#endif
void my_usart_init(uint16_t);

void my_usart_write(const char*, uint8_t);

void my_usart_flush();
#ifdef __cplusplus
}
#endif