add_library(stm32_spl STATIC
    stm32f4/STM32F4xx_StdPeriph_Driver/src/misc.c
    stm32f4/STM32F4xx_StdPeriph_Driver/src/stm32f4xx_rcc.c
    stm32f4/STM32F4xx_StdPeriph_Driver/src/stm32f4xx_gpio.c
)
target_include_directories(stm32_spl SYSTEM PUBLIC
    "${CMAKE_CURRENT_SOURCE_DIR}/stm32f4/CMSIS/Include"
    "${CMAKE_CURRENT_SOURCE_DIR}/stm32f4/CMSIS/Device/ST/STM32F4xx/Include"
    "${CMAKE_CURRENT_SOURCE_DIR}/stm32f4/STM32F4xx_StdPeriph_Driver/inc")
target_include_directories(stm32_spl PUBLIC "${PROJECT_SOURCE_DIR}/platform/stm32f407/include")
target_compile_definitions(stm32_spl PUBLIC STM32F40_41xxx USE_STDPERIPH_DRIVER
    HSE_VALUE=${COURSE_HSE_HZ}U "$<$<CONFIG:Debug>:USE_FULL_ASSERT>")
target_link_libraries(stm32_spl PUBLIC mcu_options)
