course_add_firmware(app "${COURSE_APP_BASE}" "${COURSE_APP_SIZE}")
target_sources(app PRIVATE main.c)
target_compile_definitions(app PRIVATE COURSE_APP_VERSION=1)
