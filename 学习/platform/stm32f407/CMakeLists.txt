add_library(board_support STATIC board.c)
target_include_directories(board_support PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include")
target_link_libraries(board_support PUBLIC stm32_spl)
