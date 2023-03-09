#pragma once

#include <stdint.h>


typedef enum
{
  my_timer__return_status_ok,
  my_timer__return_status_bad_parameter,
  my_timer__return_status_other
}
my_timer__return_status;

#ifdef __cplusplus
extern "C"
{
#endif
void my_timer__init();

my_timer__return_status my_timer__wait(uint16_t);
#ifdef __cplusplus
}
#endif