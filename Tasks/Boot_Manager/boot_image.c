#include "boot_image.h"
#include "boot_config.h"
#include "bsp_flash.h"
#include "AES.h"
#include <string.h>

#define IMAGE_COMMIT 0x31474D49U /* 'IMG1', little endian */

typedef struct {
    uint32_t length;
    uint32_t crc32;
    uint32_t length_inverse;
    uint32_t commit;
} ImageRecord;
_Static_assert(sizeof(ImageRecord) == BOOT_RECORD_SIZE, "Record layout mismatch");

/* The reference AES API takes a mutable key pointer. */
static uint8_t step2_key[] = BOOT_AES_KEY_ASCII;
static uint32_t package_length;
static uint32_t bytes_staged;
static uint32_t package_crc;
static bool package_ready;

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
    /* MSP is the Cortex-M4 main stack pointer. Vector word 1 gives the
     * reset address, with the Thumb bit set and the target in this App. */
    return stack > 0x20000000U && stack <= 0x20020000U && !(stack & 7U) &&
           (reset & 1U) && (reset & ~1U) >= BOOT_APP_BASE + 8U &&
           (reset & ~1U) < BOOT_APP_BASE + size;
}

bool Boot_ImageBegin(uint32_t size)
{
    package_ready = false;
    package_length = 0U;
    bytes_staged = 0U;
    package_crc = UINT32_MAX;
    if (size < BOOT_PACKAGE_HEADER_SIZE + BOOT_AES_BLOCK_SIZE ||
        size > BOOT_PACKAGE_MAX || size % BOOT_AES_BLOCK_SIZE != 0U) {
        return false;
    }
    package_length = size;
    return true;
}

bool Boot_ImageAppend(const uint8_t *data, uint32_t count)
{
    if (count == 0U || count > package_length - bytes_staged || count % BOOT_AES_BLOCK_SIZE != 0U) {
        return false;
    }
    if (bytes_staged == 0U && !BSP_FlashEraseStaging(package_length)) {
        return false;
    }
    if (!BSP_FlashWrite(BOOT_STAGE_BASE + bytes_staged, data, count)) {
        return false;
    }
    package_crc = crc_update(package_crc, data, count);
    bytes_staged += count;
    return true;
}

bool Boot_ImageFinish(void)
{
    const uint8_t *staging = (const uint8_t *)BOOT_STAGE_BASE;
    package_ready = package_length != 0U && bytes_staged == package_length &&
                    crc_update(UINT32_MAX, staging, package_length) == package_crc;
    return package_ready;
}

bool Boot_ImageInstall(void)
{
    if (!package_ready) {
        return false;
    }
    package_ready = false;

    const uint8_t *ciphertext = (const uint8_t *)BOOT_STAGE_BASE;
    uint8_t iv[BOOT_AES_BLOCK_SIZE];
    uint8_t block[BOOT_AES_BLOCK_SIZE];
    memcpy(iv, BOOT_AES_IV_ASCII, sizeof iv);
    memcpy(block, ciphertext, sizeof block);
    BOOT_AES_DECRYPT_BLOCK(iv, block, step2_key);

    const uint8_t *length_field = block + BOOT_PACKAGE_LENGTH_OFFSET;
    const uint32_t length = (uint32_t)length_field[0] | ((uint32_t)length_field[1] << 8) |
                            ((uint32_t)length_field[2] << 16) | ((uint32_t)length_field[3] << 24);
    if (length < 8U || length > BOOT_IMAGE_MAX) {
        return false;
    }
    const uint32_t padded_length =
        ((length + BOOT_AES_BLOCK_SIZE - 1U) / BOOT_AES_BLOCK_SIZE) * BOOT_AES_BLOCK_SIZE;
    if (BOOT_PACKAGE_HEADER_SIZE + padded_length != package_length) {
        return false;
    }

    /* Decode the first body block and check both vectors before erasing App. */
    ciphertext += BOOT_PACKAGE_HEADER_SIZE;
    memcpy(block, ciphertext, sizeof block);
    BOOT_AES_DECRYPT_BLOCK(iv, block, step2_key);
    if (!vectors_valid(block, length) || !BSP_FlashEraseImage(length)) {
        return false;
    }

    uint32_t crc = UINT32_MAX;
    for (uint32_t offset = 0U; offset < length; offset += BOOT_AES_BLOCK_SIZE) {
        if (offset != 0U) {
            memcpy(block, ciphertext + offset, sizeof block);
            BOOT_AES_DECRYPT_BLOCK(iv, block, step2_key);
        }
        uint32_t count = length - offset;
        if (count > BOOT_AES_BLOCK_SIZE) {
            count = BOOT_AES_BLOCK_SIZE;
        }
        if (!BSP_FlashWrite(BOOT_APP_BASE + offset, block, count)) {
            return false;
        }
        crc = crc_update(crc, block, count);
    }

    if (crc_update(UINT32_MAX, (const uint8_t *)BOOT_APP_BASE, length) != crc) {
        return false;
    }
    ImageRecord record = {length, ~crc, ~length, IMAGE_COMMIT};
    /* The writer programs in address order, with the commit word last. */
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
