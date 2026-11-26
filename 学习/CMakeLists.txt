cmake_minimum_required(VERSION 3.22)
# Read the toolchain from the selected preset BEFORE project() enables languages.
project(ota_course VERSION 0.1.0 LANGUAGES C ASM)

if(NOT CMAKE_C_COMPILER_ID STREQUAL "GNU" OR CMAKE_C_COMPILER_VERSION VERSION_LESS 11)
    message(FATAL_ERROR "Course firmware requires Arm GNU GCC >= 11 (READONLY linker sections).")
endif()
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug CACHE STRING "Build configuration")
endif()

# Default lesson is heartbeat. JumpDebug enables the handoff lesson explicitly.
option(COURSE_ENABLE_APP_JUMP "Try App vectors before Bootloader heartbeat" OFF)
include(cmake/FlashLayout.cmake)
include(cmake/McuOptions.cmake)
include(cmake/Firmware.cmake)

add_subdirectory(third_party)
add_subdirectory(platform/stm32f407)
add_subdirectory(components)
add_subdirectory(firmware/bootloader)
add_subdirectory(firmware/app)

message(STATUS "Bootloader: ${COURSE_FLASH_BASE}, ${COURSE_BOOT_SIZE} bytes")
message(STATUS "App: ${COURSE_APP_BASE}, ${COURSE_APP_SIZE} bytes")
message(STATUS "App jump enabled: ${COURSE_ENABLE_APP_JUMP}")
