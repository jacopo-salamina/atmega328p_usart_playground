#pragma once

#include <stdbool.h>
#include <stdint.h>
#include <avr/pgmspace.h>
#include "return_status.h"


// These macros are provided in order to handle string literals more easily.
#define MY_USART__WRITE_CONST(str) my_usart__write(str, (sizeof str) - 1)
#define MY_USART__WRITE_CONST_F(str) \
my_usart__write_from_pgm(PSTR(str), (sizeof str) - 1)

#ifdef __cplusplus
extern "C"
{
#endif
/**
 * Initialize the USART module with a specific baud rate.
 */
return_status my_usart__init(uint16_t);

/**
 * Reads a string from SRAM and writes it in a non-blocking fashion (store its
 * contents inside an internal buffer).
 */
return_status my_usart__write_from_sram(const char *, uint8_t);

/**
 * Same as my_usart__write_from_sram, but the string is read from the program
 * memory.
 */
return_status my_usart__write_from_pgm(PGM_P, uint8_t);

/**
 * Checks whether there's an ongoing transmission.
 * 
 * Note: this method lacks any atomicity safeguards, and is expected to be run
 * within an outer atomic block. This is intentional, as the main loop needs to
 * atomically check the state of the task manager, the USART module, the timer
 * and the ADC module.
 */
bool my_usart__is_transmission_active();
#ifdef __cplusplus
}
#endif