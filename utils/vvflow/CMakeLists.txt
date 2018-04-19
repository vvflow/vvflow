cmake_minimum_required (VERSION 3.0)
project (vvflow)

include_directories(${LIBVVHD_INCLUDE_DIRS})

add_executable(vvflow vvflow.cpp)
target_link_libraries(vvflow vvhd)

install (TARGETS vvflow DESTINATION bin)
