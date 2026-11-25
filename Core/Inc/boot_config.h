#ifndef BOOT_CONFIG_H
#define BOOT_CONFIG_H

/* Keep the Flash layout synchronized with System/Linker/bootloader.ld. */
#define BOOT_FLASH_BASE         0x08000000UL
#define BOOT_FLASH_SIZE         (32UL * 1024UL)
#define BOOT_APP_BASE           0x08008000UL
#define BOOT_APP_SIZE           (480UL * 1024UL)
/* Four words in sector 7 form the persistent image record. */
#define BOOT_RECORD_SIZE        16UL
#define BOOT_IMAGE_MAX          (BOOT_APP_SIZE - BOOT_RECORD_SIZE)
#define BOOT_RECORD_BASE        (BOOT_APP_BASE + BOOT_IMAGE_MAX)
#define BOOT_SYSTEM_CLOCK_HZ    168000000UL
#define BOOT_HEARTBEAT_MS       500UL

_Static_assert(BOOT_FLASH_BASE + BOOT_FLASH_SIZE == BOOT_APP_BASE,
               "Bootloader and App must be adjacent");
_Static_assert(BOOT_APP_BASE + BOOT_APP_SIZE == 0x08080000UL,
               "App must fit STM32F407ZE's 512 KiB Flash");
#endif
