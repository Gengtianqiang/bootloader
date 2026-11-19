#ifndef BOOT_CONFIG_H
#define BOOT_CONFIG_H

/* Keep the Flash layout synchronized with linker/bootloader.ld. */
#define BOOT_FLASH_BASE         0x08000000UL
#define BOOT_FLASH_SIZE         (64UL * 1024UL)
#define BOOT_APP_BASE           0x08010000UL
#define BOOT_APP_SIZE           (448UL * 1024UL)
#define BOOT_SYSTEM_CLOCK_HZ    168000000UL
#define BOOT_HEARTBEAT_MS       500UL

_Static_assert(BOOT_FLASH_BASE + BOOT_FLASH_SIZE == BOOT_APP_BASE,
               "Bootloader and App must be adjacent");
_Static_assert(BOOT_APP_BASE + BOOT_APP_SIZE == 0x08080000UL,
               "App must fit STM32F407ZE's 512 KiB Flash");
#endif
