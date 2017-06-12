cmake_minimum_required (VERSION 3.0)
set (CMAKE_CXX_STANDARD 11)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/Modules/")
project (vvhd)

# add_subdirectory(libvvhd)
# add_subdirectory(utils/vvflow)
# add_subdirectory(utils/vvcompose)
add_subdirectory(utils/vvcompose2)
