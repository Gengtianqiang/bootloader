#include "board.h"
#include "stm32f4xx.h"
#include "course_layout.h"
static volatile uint32_t tick_ms;
volatile uint32_t g_course_fault;

void Board_Init(void)
{
    GPIO_InitTypeDef gpio;
    SystemCoreClockUpdate();
    if (SystemCoreClock != COURSE_SYSCLK_HZ) {
        Board_Fault(1U);
    }
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOF, ENABLE);
    (void)RCC->AHB1ENR;
    GPIO_ResetBits(GPIOF, GPIO_Pin_9 | GPIO_Pin_10);
    GPIO_StructInit(&gpio);
    gpio.GPIO_Pin = GPIO_Pin_9 | GPIO_Pin_10;
    gpio.GPIO_Mode = GPIO_Mode_OUT;
    gpio.GPIO_Speed = GPIO_Speed_2MHz;
    GPIO_Init(GPIOF, &gpio);
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_4);
    tick_ms = 0U;
    if (SysTick_Config(SystemCoreClock / 1000U) != 0U) {
        Board_Fault(2U);
    }
}

void Board_LedToggle(uint32_t led)
{
    GPIO_ToggleBits(GPIOF, led == BOARD_LED_BOOT ? GPIO_Pin_9 : GPIO_Pin_10);
}
void Board_Tick(void) { ++tick_ms; }
void Board_DelayMs(uint32_t duration)
{
    uint32_t start = tick_ms;
    while ((uint32_t)(tick_ms - start) < duration) {
        __WFI();
    }
}
_Noreturn void Board_Fault(uint32_t reason)
{
    g_course_fault = reason;
    __disable_irq();
    for (;;) { __NOP(); }
}
#ifdef USE_FULL_ASSERT
void assert_failed(uint8_t *file, uint32_t line)
{
    (void)file; (void)line;
    Board_Fault(3U);
}
#endif
