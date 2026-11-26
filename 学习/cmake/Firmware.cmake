# Functions keep per-image settings scoped to each executable, rather than globals.
function(course_add_firmware target image_origin image_size)
    set(IMAGE_ORIGIN "${image_origin}")
    set(IMAGE_SIZE "${image_size}")
    set(_script "${PROJECT_BINARY_DIR}/generated/${target}.ld")
    configure_file("${PROJECT_SOURCE_DIR}/linker/firmware.ld.in" "${_script}" @ONLY)

    add_executable(${target}
        "${PROJECT_SOURCE_DIR}/startup/startup_stm32f40_41xxx.s"
        "${PROJECT_SOURCE_DIR}/platform/stm32f407/system_stm32f4xx.c"
        "${PROJECT_SOURCE_DIR}/platform/stm32f407/interrupts.c"
    )
    # Keep interrupt overrides in the executable: weak startup handlers can stop
    # otherwise-unreferenced handlers being extracted from a static library.
    set_target_properties(${target} PROPERTIES SUFFIX ".elf"
        RUNTIME_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}/firmware/${target}")
    math(EXPR _vector_offset "${image_origin} - ${COURSE_FLASH_BASE}" OUTPUT_FORMAT HEXADECIMAL)
    target_compile_definitions(${target} PRIVATE COURSE_VECTOR_OFFSET=${_vector_offset})
    target_link_libraries(${target} PRIVATE board_support m)
    target_link_options(${target} PRIVATE
        "-T${_script}" --specs=nano.specs --specs=nosys.specs
        "LINKER:-Map,${PROJECT_BINARY_DIR}/firmware/${target}/${target}.map"
        LINKER:--gc-sections LINKER:--print-memory-usage)
    set_property(TARGET ${target} APPEND PROPERTY LINK_DEPENDS "${_script}")
    add_custom_command(TARGET ${target} POST_BUILD
        COMMAND "${CMAKE_OBJCOPY}" -O binary "$<TARGET_FILE:${target}>"
            "$<TARGET_FILE_DIR:${target}>/${target}.bin"
        COMMAND "${CMAKE_OBJCOPY}" -O ihex "$<TARGET_FILE:${target}>"
            "$<TARGET_FILE_DIR:${target}>/${target}.hex"
        COMMAND "${CMAKE_SIZE}" "$<TARGET_FILE:${target}>"
        COMMENT "Exporting ${target}.bin and ${target}.hex" VERBATIM)
    set_property(TARGET ${target} APPEND PROPERTY ADDITIONAL_CLEAN_FILES
        "${PROJECT_BINARY_DIR}/firmware/${target}/${target}.bin"
        "${PROJECT_BINARY_DIR}/firmware/${target}/${target}.hex"
        "${PROJECT_BINARY_DIR}/firmware/${target}/${target}.map")
endfunction()
