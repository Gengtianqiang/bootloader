cmake_minimum_required(VERSION 3.22)
project(bootloader VERSION 0.1.0 LANGUAGES C ASM)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    set(CMAKE_BUILD_TYPE Debug CACHE STRING "Build configuration" FORCE)
endif()

# The firmware and every library must use the same CPU/FPU ABI.
add_library(mcu_options INTERFACE)
target_compile_options(mcu_options INTERFACE
    -mcpu=cortex-m4 -mthumb -mfpu=fpv4-sp-d16 -mfloat-abi=hard
    $<$<COMPILE_LANGUAGE:C>:-ffunction-sections>
    $<$<COMPILE_LANGUAGE:C>:-fdata-sections>
    $<$<COMPILE_LANGUAGE:C>:-Wall>
    $<$<COMPILE_LANGUAGE:C>:-Wextra>
    $<$<CONFIG:Debug>:-Og>
    $<$<CONFIG:Debug>:-g3>
    $<$<CONFIG:Release>:-Os>
    $<$<CONFIG:Release>:-g0>
)
target_link_options(mcu_options INTERFACE
    -mcpu=cortex-m4 -mthumb -mfpu=fpv4-sp-d16 -mfloat-abi=hard
)

add_subdirectory(Libraries)
add_executable(bootloader
    startup/startup_stm32f40_41xxx.s
    User/Src/main.c
    User/Src/system_stm32f4xx.c
    User/Src/stm32f4xx_it.c
    BSP/Src/bsp_led.c
    BSP/Src/bsp_tick.c
)
set_target_properties(bootloader PROPERTIES SUFFIX ".elf")
target_include_directories(bootloader PRIVATE User/Inc BSP/Inc)
target_link_libraries(bootloader PRIVATE stm32_spl m)

# Linker scripts belong to firmware targets, not to the common toolchain.
set(BOOT_LINKER_SCRIPT "${CMAKE_CURRENT_SOURCE_DIR}/linker/bootloader.ld")
target_link_options(bootloader PRIVATE
    "-T${BOOT_LINKER_SCRIPT}"
    --specs=nano.specs
    --specs=nosys.specs
    "LINKER:-Map,${CMAKE_CURRENT_BINARY_DIR}/bootloader.map"
    LINKER:--gc-sections
    LINKER:--print-memory-usage
)
set_property(TARGET bootloader APPEND PROPERTY LINK_DEPENDS "${BOOT_LINKER_SCRIPT}")
add_custom_command(TARGET bootloader POST_BUILD
    COMMAND "${CMAKE_OBJCOPY}" -O binary
        "$<TARGET_FILE:bootloader>" "${CMAKE_CURRENT_BINARY_DIR}/bootloader.bin"
    COMMAND "${CMAKE_OBJCOPY}" -O ihex
        "$<TARGET_FILE:bootloader>" "${CMAKE_CURRENT_BINARY_DIR}/bootloader.hex"
    COMMAND "${CMAKE_SIZE}" "$<TARGET_FILE:bootloader>"
    BYPRODUCTS
        "${CMAKE_CURRENT_BINARY_DIR}/bootloader.bin"
        "${CMAKE_CURRENT_BINARY_DIR}/bootloader.hex"
    COMMENT "Exporting bootloader.bin and bootloader.hex"
    VERBATIM
)


# 清理规则和提示信息
set_property(TARGET bootloader APPEND PROPERTY ADDITIONAL_CLEAN_FILES
    "${CMAKE_CURRENT_BINARY_DIR}/bootloader.map"
)
