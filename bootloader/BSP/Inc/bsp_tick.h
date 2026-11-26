#ifndef BSP_TICK_H
#define BSP_TICK_H
#include <stdint.h>
uint32_t BSP_Tick_Init(void);
void BSP_Tick_Inc(void);
uint32_t BSP_GetTick(void);
void BSP_DelayMs(uint32_t duration);
#endif
