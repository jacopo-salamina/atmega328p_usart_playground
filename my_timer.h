#pragma once

#include <stdint.h>


#ifdef __cplusplus
extern "C"
{
#endif
void my_timer_init();

void my_timer_wait(uint16_t);
#ifdef __cplusplus
}
#endif