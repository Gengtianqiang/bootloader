#ifndef COURSE_BOARD_H
#define COURSE_BOARD_H
#include <stdint.h>
enum { BOARD_LED_BOOT = 0, BOARD_LED_APP = 1 };
/* Board assumption: HSE 8 MHz; PF9/PF10 LEDs. Check your schematic. */
void Board_Init(void);
void Board_LedToggle(uint32_t led);
void Board_DelayMs(uint32_t duration);
void Board_Tick(void);
_Noreturn void Board_Fault(uint32_t reason);
extern volatile uint32_t g_course_fault;
#endif
