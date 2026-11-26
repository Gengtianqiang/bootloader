#include "board.h"
#include "stm32f4xx.h"
#include "course_layout.h"
volatile uint32_t g_app_heartbeat;
volatile uint32_t g_app_version = COURSE_APP_VERSION;
int main(void)
{
    /* SystemInit already set the App-specific VTOR, before main. */
    if (SCB->VTOR != COURSE_APP_BASE) {
        Board_Fault(20U);
    }
    Board_Init();
    for (;;) {
        ++g_app_heartbeat;
        Board_LedToggle(BOARD_LED_APP);
        Board_DelayMs(200U);
    }
}
