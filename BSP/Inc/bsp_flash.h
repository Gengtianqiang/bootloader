#ifndef BSP_FLASH_H
#define BSP_FLASH_H
#include <stdbool.h>
#include <stdint.h>

/* All operations are confined to App sectors 2..7, including metadata. */
bool BSP_FlashEraseImage(uint32_t length);
bool BSP_FlashWrite(uint32_t address, const uint8_t *data, uint32_t length);
#endif
