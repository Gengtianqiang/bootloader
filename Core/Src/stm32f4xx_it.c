#include "stm32f4xx_it.h"
#include "main.h"
#include "bsp_tick.h"

void NMI_Handler(void)
{
    Error_Handler(BOOT_ERROR_NMI);
}
void HardFault_Handler(void)
{
    Error_Handler(BOOT_ERROR_HARDFAULT);
}
void MemManage_Handler(void)
{
    Error_Handler(BOOT_ERROR_MEMMANAGE);
}
void BusFault_Handler(void)
{
    Error_Handler(BOOT_ERROR_BUSFAULT);
}
void UsageFault_Handler(void)
{
    Error_Handler(BOOT_ERROR_USAGEFAULT);
}
void SVC_Handler(void)
{
}
void DebugMon_Handler(void)
{
}
void PendSV_Handler(void)
{
}
void SysTick_Handler(void)
{
    BSP_Tick_Inc();
}
