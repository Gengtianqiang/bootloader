#include "boot_image.h"
#include "boot_config.h"
#include "bsp_flash.h"
#include <string.h>

#define IMAGE_COMMIT 0x31474D49U /* 'IMG1', little endian */
typedef struct {
    uint32_t length;
    uint32_t crc32;
    uint32_t length_inverse;
    uint32_t commit;
} ImageRecord;
_Static_assert(sizeof(ImageRecord) == BOOT_RECORD_SIZE, "Record layout mismatch");

static uint32_t total, received, running_crc;

static uint32_t crc_update(uint32_t crc, const uint8_t *data, uint32_t size)
{
    while (size--) {
        crc ^= *data++;
        for (unsigned i = 0; i < 8; ++i) {
            crc = (crc >> 1) ^ ((0U - (crc & 1U)) & 0xEDB88320U);
        }
    }
    return crc;
}

static bool vectors_valid(const uint8_t *data, uint32_t size)
{
    uint32_t stack, reset;
    memcpy(&stack, data, 4U);
    memcpy(&reset, data + 4U, 4U);
    /* MSP is the Cortex-M4 main stack pointer. The second vector is a
     * Thumb code address, with bit 0 set and the target inside this file. */
    return stack > 0x20000000U && stack <= 0x20020000U && !(stack & 7U) &&
           (reset & 1U) && (reset & ~1U) >= BOOT_APP_BASE + 8U &&
           (reset & ~1U) < BOOT_APP_BASE + size;
}

bool Boot_ImageBegin(uint32_t size)
{
    received = 0;
    running_crc = UINT32_MAX;
    total = size;
    return size >= 8U && size <= BOOT_IMAGE_MAX;
}

bool Boot_ImageAppend(const uint8_t *data, uint32_t count)
{
    if (total > BOOT_IMAGE_MAX || count == 0U || count > total - received) return false;
    if (received == 0U && (count < 8U || !vectors_valid(data, total) ||
                           !BSP_FlashEraseImage(total))) return false;
    if (!BSP_FlashWrite(BOOT_APP_BASE + received, data, count)) return false;
    running_crc = crc_update(running_crc, data, count);
    received += count;
    return true;
}

bool Boot_ImageFinish(void)
{
    if (received != total || total < 8U || total > BOOT_IMAGE_MAX) return false;
    ImageRecord record = {total, ~running_crc, ~total, IMAGE_COMMIT};
    uint32_t from_flash = ~crc_update(UINT32_MAX, (const uint8_t *)BOOT_APP_BASE, total);
    if (from_flash != record.crc32) return false;
    /* BSP programs words in address order. Commit is the final word. */
    return BSP_FlashWrite(BOOT_RECORD_BASE, (const uint8_t *)&record, sizeof record);
}

bool Boot_ImageValid(void)
{
    const volatile ImageRecord *record = (const volatile ImageRecord *)BOOT_RECORD_BASE;
    const uint8_t *base = (const uint8_t *)BOOT_APP_BASE;
    uint32_t size = record->length;
    return record->commit == IMAGE_COMMIT && size >= 8U && size <= BOOT_IMAGE_MAX &&
           record->length_inverse == ~size && vectors_valid(base, size) &&
           record->crc32 == ~crc_update(UINT32_MAX, base, size);
}
