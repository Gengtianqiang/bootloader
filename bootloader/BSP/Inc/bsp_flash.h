#ifndef BSP_FLASH_H
#define BSP_FLASH_H
#include <stdbool.h>
#include <stdint.h>

bool BSP_FlashEraseImage(uint32_t length);
bool BSP_FlashEraseStaging(uint32_t length);
bool BSP_FlashWrite(uint32_t address, const uint8_t *data, uint32_t length);
#endif
