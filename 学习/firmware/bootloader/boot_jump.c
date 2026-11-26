#include "boot_jump.h"
#include "image_vectors.h"
#include "stm32f4xx.h"
#include "course_layout.h"
extern void Boot_EnterApp(uint32_t stack_top, uint32_t reset_handler)
    __attribute__((noreturn));
int Boot_AppVectorsValid(void)
{
    return Image_VectorsValid(*(volatile const uint32_t *)COURSE_APP_BASE,
        *(volatile const uint32_t *)(COURSE_APP_BASE + 4U));
}
void Boot_JumpToApp(void)
{
    uint32_t stack_top = *(volatile const uint32_t *)COURSE_APP_BASE;
    uint32_t reset_handler = *(volatile const uint32_t *)(COURSE_APP_BASE + 4U);
    uint32_t bank;
    if (__get_IPSR() != 0U || (__get_CONTROL() & 1U) != 0U ||
        !Image_VectorsValid(stack_top, reset_handler)) {
        return;
    }
    __disable_irq();
    SysTick->CTRL = 0U; SysTick->LOAD = 0U; SysTick->VAL = 0U;
    /* Current firmware uses no UART, DMA or watchdog. Stop future IRQ sources
     * and DMA channels here before changing the memory/clock environment. */
    RCC_AHB1PeriphResetCmd(RCC_AHB1Periph_GPIOF, ENABLE);
    RCC_AHB1PeriphResetCmd(RCC_AHB1Periph_GPIOF, DISABLE);
    RCC_HSICmd(ENABLE);
    while (RCC_GetFlagStatus(RCC_FLAG_HSIRDY) == RESET) { }
    RCC_SYSCLKConfig(RCC_SYSCLKSource_HSI);
    while (RCC_GetSYSCLKSource() != 0x00U) { }
    RCC_DeInit();
    for (bank = 0U; bank < ((SCnSCB->ICTR & 0xFUL) + 1U); ++bank) {
        NVIC->ICER[bank] = 0xFFFFFFFFUL;
        NVIC->ICPR[bank] = 0xFFFFFFFFUL;
    }
    SCB->ICSR = SCB_ICSR_PENDSTCLR_Msk | SCB_ICSR_PENDSVCLR_Msk;
    SCB->VTOR = COURSE_APP_BASE;
    __DSB(); __ISB();
    Boot_EnterApp(stack_top, reset_handler);
}
