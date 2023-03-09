#pragma once

#include <stdbool.h>


#ifdef __cplusplus
extern "C"
{
#endif
void my_task__queue_new(void (*)(void*), void*);

bool my_task__try_to_run_next();
#ifdef __cplusplus
}
#endif