#ifndef BOOT_IMAGE_H
#define BOOT_IMAGE_H
#include <stdbool.h>
#include <stdint.h>

bool Boot_ImageBegin(uint32_t size);
bool Boot_ImageAppend(const uint8_t *data, uint32_t count);
bool Boot_ImageFinish(void);
bool Boot_ImageInstall(void);
bool Boot_ImageValid(void);
#endif
