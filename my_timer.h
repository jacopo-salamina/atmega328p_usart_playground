#pragma once

#include <stdint.h>


#ifdef __cplusplus
extern "C"
{
#endif
void my_timer_init();

void my_timer_set_delay(uint16_t);

void my_timer_wait();
#ifdef __cplusplus
}
#endif