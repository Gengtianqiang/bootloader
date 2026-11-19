#ifndef MAIN_H
#define MAIN_H
#include "stm32f4xx.h"

typedef enum {
    BOOT_ERROR_NONE = 0,
    BOOT_ERROR_CLOCK = 1,
    BOOT_ERROR_SYSTICK = 2,
    BOOT_ERROR_ASSERT = 3,
    BOOT_ERROR_NMI = 4,
    BOOT_ERROR_HARDFAULT = 5,
    BOOT_ERROR_MEMMANAGE = 6,
    BOOT_ERROR_BUSFAULT = 7,
    BOOT_ERROR_USAGEFAULT = 8
} Boot_Error;

/* Inspect this variable in the debugger if startup or an exception fails. */
extern volatile uint32_t g_boot_error;
_Noreturn void Error_Handler(Boot_Error error);
#endif
