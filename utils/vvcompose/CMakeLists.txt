cmake_minimum_required (VERSION 3.0)
set (CMAKE_CXX_STANDARD 11)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/Modules/")
project (vvcompose)

add_subdirectory(./../../libvvhd libvvhd)

SET(CMAKE_EXE_LINKER_FLAGS "-static")
include_directories(${LIBVVHD_INCLUDE_DIR})
add_executable(vvcompose vvcompose.cpp)
set_target_properties(vvcompose PROPERTIES LINK_SEARCH_START_STATIC 1)
set_target_properties(vvcompose PROPERTIES LINK_SEARCH_END_STATIC 1)
target_link_libraries(vvcompose ${LIBVVHD_LIBRARIES})
