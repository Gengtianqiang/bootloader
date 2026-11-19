#include "bsp_led.h"
#include "stm32f4xx.h"

void BSP_LED_Init(void)
{
    GPIO_InitTypeDef gpio;

    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOF, ENABLE);
    (void)RCC->AHB1ENR;
    GPIO_ResetBits(GPIOF, GPIO_Pin_9 | GPIO_Pin_10);
    GPIO_StructInit(&gpio);
    gpio.GPIO_Pin = GPIO_Pin_9 | GPIO_Pin_10;
    gpio.GPIO_Mode = GPIO_Mode_OUT;
    gpio.GPIO_Speed = GPIO_Speed_2MHz;
    gpio.GPIO_OType = GPIO_OType_PP;
    gpio.GPIO_PuPd = GPIO_PuPd_NOPULL;
    GPIO_Init(GPIOF, &gpio);
}

void BSP_LED_ToggleAll(void)
{
    GPIO_ToggleBits(GPIOF, GPIO_Pin_9 | GPIO_Pin_10);
}
