#ifndef STM32F4XX_CONF_H
#define STM32F4XX_CONF_H

/* Add the peripheral header here and its .c file in Drivers/CMakeLists.txt. */
#include "stm32f4xx_gpio.h"
#include "stm32f4xx_rcc.h"
#include "stm32f4xx_usart.h"
#include "stm32f4xx_dma.h"
#include "stm32f4xx_tim.h"
#include "stm32f4xx_flash.h"
#include "misc.h"

#ifdef USE_FULL_ASSERT
void assert_failed(uint8_t *file, uint32_t line);
#define assert_param(expr) ((expr) ? (void)0 : assert_failed((uint8_t *)__FILE__, (uint32_t)__LINE__))
#else
#define assert_param(expr) ((void)0)
#endif
#endif
