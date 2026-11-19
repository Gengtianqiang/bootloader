#include "bsp_tick.h"
#include "stm32f4xx.h"

static volatile uint32_t tick_ms;

uint32_t BSP_Tick_Init(void)
{
    tick_ms = 0U;
    return SysTick_Config(SystemCoreClock / 1000U);
}
void BSP_Tick_Inc(void)
{
    ++tick_ms;
}
uint32_t BSP_GetTick(void)
{
    return tick_ms;
}
void BSP_DelayMs(uint32_t duration)
{
    const uint32_t start = BSP_GetTick();
    /* Unsigned subtraction handles the tick counter wrapping around. */
    while ((uint32_t)(BSP_GetTick() - start) < duration) {
        __WFI();
    }
}
