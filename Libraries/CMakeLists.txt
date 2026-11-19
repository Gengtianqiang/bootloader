# Initial milestone needs only RCC, GPIO, and NVIC/SysTick support.
# Add further SPL source files here when implementing another peripheral.
add_library(stm32_spl STATIC
    STM32F4xx_StdPeriph_Driver/src/misc.c
    STM32F4xx_StdPeriph_Driver/src/stm32f4xx_rcc.c
    STM32F4xx_StdPeriph_Driver/src/stm32f4xx_gpio.c
)
target_include_directories(stm32_spl SYSTEM PUBLIC
    "${CMAKE_CURRENT_SOURCE_DIR}/CMSIS/Include"
    "${CMAKE_CURRENT_SOURCE_DIR}/CMSIS/Device/ST/STM32F4xx/Include"
    "${CMAKE_CURRENT_SOURCE_DIR}/STM32F4xx_StdPeriph_Driver/inc"
)
target_include_directories(stm32_spl PUBLIC "${PROJECT_SOURCE_DIR}/User/Inc")
target_compile_definitions(stm32_spl PUBLIC
    STM32F40_41xxx
    USE_STDPERIPH_DRIVER
    HSE_VALUE=8000000U
    $<$<CONFIG:Debug>:USE_FULL_ASSERT>
)
target_link_libraries(stm32_spl PUBLIC mcu_options)
