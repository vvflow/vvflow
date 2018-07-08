cmake_minimum_required (VERSION 3.0)
project (pytest)

add_test(pytests python3 -mpytest ${CMAKE_CURRENT_SOURCE_DIR})

set(PATH_LIST
    "${CMAKE_BINARY_DIR}/utils/vvcompose/"
    "${CMAKE_BINARY_DIR}/utils/vvflow/"
    "${CMAKE_BINARY_DIR}/utils/vvplot3/"
    "${CMAKE_BINARY_DIR}/utils/vvxtract/"
    "$ENV{PATH}"
)

string(REPLACE ";" ":" PATH_LIST "${PATH_LIST}")
set_property(TEST pytests PROPERTY ENVIRONMENT
    "PATH=${PATH_LIST}"
)
