#pragma once

#include <stdbool.h>
#include <stdint.h>
#include "my_task.h"


#ifdef __cplusplus
extern "C"
{
#endif
void my_timer__init();

void my_timer__set_timeout(uint16_t, task_t);

bool my_timer__is_timeout_pending();
#ifdef __cplusplus
}
#endif