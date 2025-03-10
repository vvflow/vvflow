cmake_minimum_required (VERSION 3.0)
project (pytest)

add_test(pytests pipenv run pytest ${CMAKE_CURRENT_SOURCE_DIR})
add_test(flake8 pipenv run flake8 ${CMAKE_CURRENT_SOURCE_DIR})
add_test(black pipenv run black ${CMAKE_CURRENT_SOURCE_DIR} --check --diff)

set(PATH_LIST
    "${CMAKE_BINARY_DIR}/utils/vvflow/"
    "${CMAKE_BINARY_DIR}/utils/vvplot/"
    "${CMAKE_BINARY_DIR}/utils/vvxtract/"
    "${CMAKE_SOURCE_DIR}/utils/scripts"
    "$ENV{PATH}"
)

string(REPLACE ";" ":" PATH_LIST "${PATH_LIST}")
set_property(TEST pytests PROPERTY ENVIRONMENT
    "PATH=${PATH_LIST}"
)
