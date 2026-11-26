add_library(image_format STATIC image/image_vectors.c)
target_include_directories(image_format PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/image/include")
target_link_libraries(image_format PUBLIC mcu_options)

# Pure decision function for the later persistent-state lesson, not an installer.
add_library(ota_policy STATIC ota/ota_policy.c)
target_include_directories(ota_policy PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/ota/include")
target_link_libraries(ota_policy PUBLIC mcu_options)
