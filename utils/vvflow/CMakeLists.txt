cmake_minimum_required (VERSION 3.0)
set (CMAKE_CXX_STANDARD 11)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/Modules/")
project (vvflow)

add_subdirectory(./../../libvvhd libvvhd)

# set (VVFLOW_SOURCES ${VVFLOW_SOURCES} vvflow.cpp)

# set(HDF5_USE_STATIC_LIBRARIES 1)
# find_package(HDF5 REQUIRED)
# include_directories(${HDF5_INCLUDE_DIRS})

# add_library(vvhd STATIC ${VVFLOW_SOURCES})
message("LIBVVHD_INCLUDE_DIR: " ${LIBVVHD_INCLUDE_DIR})
message("LIBVVHD_LIBRARIES: " "${LIBVVHD_LIBRARIES}")
# message("")


# SET(CMAKE_FIND_LIBRARY_SUFFIXES ".a")
# SET(BUILD_SHARED_LIBRARIES OFF)
SET(CMAKE_EXE_LINKER_FLAGS "-static")
# SET(CMAKE_EXE_LINKER_FLAGS "-static-libgcc -static-libstdc++")
include_directories(${LIBVVHD_INCLUDE_DIR})
add_executable(vvflow vvflow.cpp)
set_target_properties(vvflow PROPERTIES LINK_SEARCH_START_STATIC 1)
set_target_properties(vvflow PROPERTIES LINK_SEARCH_END_STATIC 1)
target_link_libraries(vvflow ${LIBVVHD_LIBRARIES})
