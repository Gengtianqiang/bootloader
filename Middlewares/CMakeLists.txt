add_library(ymodem STATIC Ymodem/ymodem.c)
target_include_directories(ymodem PUBLIC Ymodem)
target_link_libraries(ymodem PRIVATE mcu_options)

add_library(boot_log STATIC
    easylogger/src/elog.c
    easylogger/src/elog_utils.c
    easylogger/port/elog_port.c
    Rtt/SEGGER_RTT.c
)
target_include_directories(boot_log PUBLIC easylogger/inc Rtt)
target_compile_definitions(boot_log PUBLIC RTT_USE_ASM=0)
target_link_libraries(boot_log PRIVATE mcu_options)
