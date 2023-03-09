#pragma once

#include <stdint.h>


#ifdef __cplusplus
extern "C"
{
#endif
void my_timer__init();

void my_timer__wait(uint16_t);
#ifdef __cplusplus
}
#endif