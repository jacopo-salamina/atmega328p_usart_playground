#pragma once

#include <stdbool.h>
#include <stdint.h>


#ifdef __cplusplus
extern "C"
{
#endif
void my_timer__init();

void my_timer__set_timeout(uint16_t, void (*)(void*), void*);

bool my_timer__is_timeout_pending();
#ifdef __cplusplus
}
#endif