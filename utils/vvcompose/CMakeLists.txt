cmake_minimum_required (VERSION 3.0)
set (CMAKE_CXX_STANDARD 11)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/Modules/")
project (vvcompose)

add_subdirectory(./../../libvvhd libvvhd)
include_directories(${LIBVVHD_INCLUDE_DIR})

add_executable(vvcompose vvcompose.cpp)
target_link_libraries(vvcompose ${LIBVVHD_LIBRARIES})

install (TARGETS vvcompose DESTINATION bin)
