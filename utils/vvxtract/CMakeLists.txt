cmake_minimum_required (VERSION 3.0)
set (CMAKE_CXX_STANDARD 11)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/Modules/")
project (vvxtract)

add_subdirectory(./../../libvvhd libvvhd)
include_directories(${LIBVVHD_INCLUDE_DIR})

add_executable(vvxtract vvxtract.cpp)
target_link_libraries(vvxtract ${LIBVVHD_LIBRARIES})

install (TARGETS vvxtract DESTINATION bin)
