cmake_minimum_required (VERSION 3.0)
project (vvflow)

add_subdirectory(./../../libvvhd libvvhd)
include_directories(${LIBVVHD_INCLUDE_DIR})

add_executable(vvflow vvflow.cpp)
target_link_libraries(vvflow vvhd)

install (TARGETS vvflow DESTINATION bin)
