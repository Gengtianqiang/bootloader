#include "board.h"
void SysTick_Handler(void) { Board_Tick(); }
void NMI_Handler(void) { Board_Fault(10U); }
void HardFault_Handler(void) { Board_Fault(11U); }
void MemManage_Handler(void) { Board_Fault(12U); }
void BusFault_Handler(void) { Board_Fault(13U); }
void UsageFault_Handler(void) { Board_Fault(14U); }
