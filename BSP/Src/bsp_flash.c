#include "bsp_flash.h"
#include "boot_config.h"
#include "stm32f4xx.h"
#include <string.h>

static const struct {
    uint32_t start;
    uint32_t sector;
} app_sectors[] = {
    {0x08008000U, FLASH_Sector_2}, {0x0800C000U, FLASH_Sector_3},
    {0x08010000U, FLASH_Sector_4}, {0x08020000U, FLASH_Sector_5},
    {0x08040000U, FLASH_Sector_6}, {0x08060000U, FLASH_Sector_7}
};

/* The F407 Flash accelerator caches both data and instructions. Keep it
 * disabled during each transaction so verification sees physical Flash. */
static uint32_t begin_write(void)
{
    uint32_t cache = FLASH->ACR;
    FLASH_DataCacheCmd(DISABLE);
    FLASH_InstructionCacheCmd(DISABLE);
    FLASH_DataCacheReset();
    FLASH_InstructionCacheReset();
    FLASH_Unlock();
    FLASH_ClearFlag(FLASH_FLAG_EOP | FLASH_FLAG_OPERR | FLASH_FLAG_WRPERR |
                    FLASH_FLAG_PGAERR | FLASH_FLAG_PGPERR | FLASH_FLAG_PGSERR);
    return cache;
}

static void end_write(uint32_t cache)
{
    FLASH_Lock();
    if (cache & FLASH_ACR_DCEN) FLASH_DataCacheCmd(ENABLE);
    if (cache & FLASH_ACR_ICEN) FLASH_InstructionCacheCmd(ENABLE);
}

bool BSP_FlashEraseImage(uint32_t length)
{
    if (length < 8U || length > BOOT_IMAGE_MAX) return false;
    uint32_t cache = begin_write();
    /* Remove the old commit record first. Sector 7 also holds payload for
     * large images. Erase it exactly once, even at maximum image size. */
    bool ok = FLASH_EraseSector(FLASH_Sector_7, VoltageRange_3) == FLASH_COMPLETE;
    for (unsigned i = 0; ok && i + 1U < sizeof app_sectors / sizeof app_sectors[0]; ++i) {
        if (app_sectors[i].start >= BOOT_APP_BASE + length) break;
        ok = FLASH_EraseSector(app_sectors[i].sector, VoltageRange_3) == FLASH_COMPLETE;
    }
    end_write(cache);
    return ok;
}

bool BSP_FlashWrite(uint32_t address, const uint8_t *data, uint32_t length)
{
    const uint32_t limit = BOOT_APP_BASE + BOOT_APP_SIZE;
    if (length == 0U || (address & 3U) || address < BOOT_APP_BASE ||
        address >= limit || length > limit - address) return false;
    uint32_t cache = begin_write();
    bool ok = true;
    while (ok && length != 0U) {
        uint32_t word = UINT32_MAX;
        uint32_t count = length < sizeof word ? length : sizeof word;
        memcpy(&word, data, count);
        ok = FLASH_ProgramWord(address, word) == FLASH_COMPLETE &&
             *(volatile const uint32_t *)address == word;
        address += sizeof word;
        data += count;
        length -= count;
    }
    end_write(cache);
    return ok;
}
