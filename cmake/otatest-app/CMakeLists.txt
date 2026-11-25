# Import the user's complete HAL application, including its own source list.
set(OTATEST_DIR "${CMAKE_SOURCE_DIR}/../otatest" CACHE PATH "Sibling HAL App source")
# otatest's project() enables C++ in addition to C, even for this C-only App.
find_program(CMAKE_CXX_COMPILER NAMES arm-none-eabi-g++
    PATHS "${ARM_TOOLCHAIN_BIN}" NO_DEFAULT_PATH REQUIRED)
set(TOOLCHAIN_LINK_LIBRARIES m)
link_libraries(mcu_options)
add_subdirectory("${OTATEST_DIR}" "${CMAKE_CURRENT_BINARY_DIR}/hal-app")

# stm32cubemx supplies these definitions to system_stm32f4xx.c as well.
target_compile_definitions(stm32cubemx INTERFACE
    USER_VECT_TAB_ADDRESS VECT_TAB_OFFSET=0x8000U)
set(app_script "${CMAKE_SOURCE_DIR}/System/Linker/otatest_app.ld")
set_target_properties(otatest PROPERTIES
    SUFFIX ".elf"
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}"
)
target_link_options(otatest PRIVATE
    "-T${app_script}" --specs=nano.specs --specs=nosys.specs
    LINKER:--gc-sections "LINKER:-Map,${CMAKE_BINARY_DIR}/otatest.map"
)
set_property(TARGET otatest APPEND PROPERTY LINK_DEPENDS "${app_script}")
add_custom_command(OUTPUT "${CMAKE_BINARY_DIR}/otatest.bin"
    COMMAND "${CMAKE_OBJCOPY}" -O binary "$<TARGET_FILE:otatest>" "${CMAKE_BINARY_DIR}/otatest.bin"
    DEPENDS otatest
    VERBATIM
)
add_custom_target(otatest_bin ALL DEPENDS "${CMAKE_BINARY_DIR}/otatest.bin")
