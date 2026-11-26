course_add_firmware(bootloader "${COURSE_FLASH_BASE}" "${COURSE_BOOT_SIZE}")
target_sources(bootloader PRIVATE main.c boot_jump.c boot_enter_app.s)
target_link_libraries(bootloader PRIVATE image_format ota_policy)
target_compile_definitions(bootloader PRIVATE
    COURSE_ENABLE_APP_JUMP=$<BOOL:${COURSE_ENABLE_APP_JUMP}>)
