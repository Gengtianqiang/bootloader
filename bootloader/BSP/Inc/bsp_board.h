#ifndef BSP_BOARD_H
#define BSP_BOARD_H
#include <stdbool.h>
#include <stdint.h>

void BSP_KeyInit(void);
bool BSP_UpdateKeyPressed(void);
void BSP_StatusTimerInit(void);
void BSP_StatusPoll(void);
void BSP_UartInit(void);
bool BSP_UartRead(uint8_t *byte, uint32_t timeout_ms);
void BSP_UartWrite(uint8_t byte);
void BSP_UartDrain(void);
void BSP_BoardDeinit(void);
#endif
