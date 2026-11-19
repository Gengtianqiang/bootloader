#include "main.h"
#include "boot_config.h"
#include "bsp_led.h"
#include "bsp_tick.h"

volatile uint32_t g_boot_error = BOOT_ERROR_NONE;

int main(void)
{
    /* SPL SystemInit() already configured the clocks in Reset_Handler. */
    SystemCoreClockUpdate();
    BSP_LED_Init();
    if (SystemCoreClock != BOOT_SYSTEM_CLOCK_HZ) {
        Error_Handler(BOOT_ERROR_CLOCK);
    }

    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_4);
    if (BSP_Tick_Init() != 0U) {
        Error_Handler(BOOT_ERROR_SYSTICK);
    }

    /* Initial milestone stays in Bootloader to verify the board heartbeat.
     * App validation/selection/transfer belong to the next milestone.
     */
    for (;;) {
        BSP_LED_ToggleAll();
        BSP_DelayMs(BOOT_HEARTBEAT_MS);
    }
}

_Noreturn void Error_Handler(Boot_Error error)
{
    g_boot_error = (uint32_t)error;
    __disable_irq();
    for (;;) {
        __NOP();
    }
}

#ifdef USE_FULL_ASSERT
void assert_failed(uint8_t *file, uint32_t line)
{
    (void)file;
    (void)line;
    Error_Handler(BOOT_ERROR_ASSERT);
}
#endif
