cmake_minimum_required (VERSION 3.0)
# set (CMAKE_CXX_STANDARD 11)
project (vvxtract)

# HDF5
find_package(HDF5 REQUIRED)
include_directories(${HDF5_INCLUDE_DIRS})

add_subdirectory(./../../libvvhd libvvhd)
include_directories(${LIBVVHD_INCLUDE_DIR})

add_executable(vvxtract vvxtract.cpp)
target_link_libraries(vvxtract vvhd)

install (TARGETS vvxtract DESTINATION bin)
