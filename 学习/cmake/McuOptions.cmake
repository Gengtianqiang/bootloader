add_library(mcu_options INTERFACE)
target_compile_options(mcu_options INTERFACE
    -mcpu=cortex-m4 -mthumb -mfpu=fpv4-sp-d16 -mfloat-abi=hard
    "$<$<COMPILE_LANGUAGE:C>:-Wall;-Wextra;-ffunction-sections;-fdata-sections;-fstack-usage>"
    "$<$<CONFIG:Debug>:-Og;-g3>"
    "$<$<CONFIG:Release>:-Os;-g0>"
)
target_link_options(mcu_options INTERFACE
    -mcpu=cortex-m4 -mthumb -mfpu=fpv4-sp-d16 -mfloat-abi=hard)
target_include_directories(mcu_options INTERFACE "${PROJECT_BINARY_DIR}/generated")
