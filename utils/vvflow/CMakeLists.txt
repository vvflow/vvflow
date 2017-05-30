cmake_minimum_required (VERSION 3.0)
set (CMAKE_CXX_STANDARD 11)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/Modules/")
project (vvflow)

add_subdirectory(./../../libvvhd libvvhd)

SET(CMAKE_EXE_LINKER_FLAGS "-static")
include_directories(${LIBVVHD_INCLUDE_DIR})
add_executable(vvflow vvflow.cpp)
set_target_properties(vvflow PROPERTIES LINK_SEARCH_START_STATIC 1)
set_target_properties(vvflow PROPERTIES LINK_SEARCH_END_STATIC 1)
target_link_libraries(vvflow ${LIBVVHD_LIBRARIES})
