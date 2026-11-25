#include "boot_jump.h"
#include "boot_config.h"
#include "boot_image.h"
#include "bsp_board.h"
#include "stm32f4xx.h"

/* Implemented in boot_jump.s: no C stack access after replacing MSP. */
extern void Boot_EnterApp(uint32_t stack_top, uint32_t reset_handler)
    __attribute__((noreturn));

uint32_t Boot_JumpToApp(void)
{
    /* IPSR is the Cortex-M4 exception number; CONTROL.nPRIV identifies
     * unprivileged Thread mode. Handoff needs privileged Thread mode. */
    if (__get_IPSR() != 0U || (__get_CONTROL() & 1U) || !Boot_ImageValid()) {
        return 0U;
    }
    uint32_t stack_top = *(volatile const uint32_t *)BOOT_APP_BASE;
    uint32_t reset_handler = *(volatile const uint32_t *)(BOOT_APP_BASE + 4U);
    uint32_t bank;

    __disable_irq();
    SysTick->CTRL = 0U;
    SysTick->LOAD = 0U;
    SysTick->VAL = 0U;

    BSP_BoardDeinit();

    /* Switch to HSI before disabling/reconfiguring the PLL. */
    RCC_HSICmd(ENABLE);
    while (RCC_GetFlagStatus(RCC_FLAG_HSIRDY) == RESET) {
    }
    RCC_SYSCLKConfig(RCC_SYSCLKSource_HSI);
    while (RCC_GetSYSCLKSource() != 0x00U) {
    }
    RCC_DeInit();

    /* Disable every implemented external IRQ and clear pending requests. */
    for (bank = 0U; bank < ((SCnSCB->ICTR & 0xFUL) + 1U); ++bank) {
        NVIC->ICER[bank] = 0xFFFFFFFFUL;
        NVIC->ICPR[bank] = 0xFFFFFFFFUL;
    }
    SCB->ICSR = SCB_ICSR_PENDSTCLR_Msk | SCB_ICSR_PENDSVCLR_Msk;
    /* VTOR is the Cortex-M4 SCB vector table base address register. */
    SCB->VTOR = BOOT_APP_BASE;
    __DSB();
    __ISB();
    Boot_EnterApp(stack_top, reset_handler);
}
