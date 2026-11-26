cmake_minimum_required(VERSION 3.22)

#
# This file is generated only once,
# and is not re-generated if converter is called multiple times.
#
# User is free to modify the file as much as necessary
#

# Setup compiler settings
set(CMAKE_C_STANDARD 11)     #使用c11标准
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS ON)


# Define the build type
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Debug")
endif()

# Set the project name
set(CMAKE_PROJECT_NAME otatest)

# Enable compile command to ease indexing with e.g. clangd
set(CMAKE_EXPORT_COMPILE_COMMANDS TRUE)

# Core project settings
project(${CMAKE_PROJECT_NAME})
message("Build type: " ${CMAKE_BUILD_TYPE})

# Enable CMake support for ASM and C languages
enable_language(C ASM)

# Create an executable object type
#创建名为 otatest 的可执行目标
add_executable(${CMAKE_PROJECT_NAME})

# Add STM32CubeMX generated sources
add_subdirectory(cmake/stm32cubemx)

# Link directories setup
target_link_directories(${CMAKE_PROJECT_NAME} PRIVATE
    # Add user defined library search paths
)

# Add sources to executable
target_sources(${CMAKE_PROJECT_NAME} PRIVATE
    # Add user sources here
)

# Add include paths
target_include_directories(${CMAKE_PROJECT_NAME} PRIVATE
    # Add user defined include paths
)

# Add project symbols (macros)
target_compile_definitions(${CMAKE_PROJECT_NAME} PRIVATE
    # Add user defined symbols
)

# Remove wrong libob.a library dependency when using cpp files
list(REMOVE_ITEM CMAKE_C_IMPLICIT_LINK_LIBRARIES ob)

# Add linked libraries
target_link_libraries(${CMAKE_PROJECT_NAME}
    stm32cubemx

    # Add user defined libraries
)

# Select the memory layout after CubeMX has declared the HAL targets.
option(OTATEST_OTA "Link the application for the OTA run partition" OFF)
set(app_linker_script "${CMAKE_CURRENT_SOURCE_DIR}/STM32F407xx_FLASH.ld")
if(OTATEST_OTA)
    set(app_linker_script "${CMAKE_CURRENT_SOURCE_DIR}/STM32F407xx_OTA.ld")
    target_compile_definitions(stm32cubemx INTERFACE
        USER_VECT_TAB_ADDRESS VECT_TAB_OFFSET=0x8000U)
endif()
target_link_options(otatest PRIVATE "-T${app_linker_script}")
set_property(TARGET otatest APPEND PROPERTY LINK_DEPENDS "${app_linker_script}")

if(OTATEST_OTA)
    set(app_bin "${CMAKE_CURRENT_BINARY_DIR}/otatest.bin")
    add_custom_command(OUTPUT "${app_bin}"
        COMMAND "${CMAKE_OBJCOPY}" -O binary "$<TARGET_FILE:otatest>" "${app_bin}"
        DEPENDS otatest
        VERBATIM
    )
    add_custom_target(otatest_bin ALL DEPENDS "${app_bin}")
endif()
