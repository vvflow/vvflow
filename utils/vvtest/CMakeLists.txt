cmake_minimum_required (VERSION 3.0)
project (vvtest)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

add_subdirectory(test)
#include_directories(${LIBVVHD_INCLUDE_DIR})

# MATHEVAL
find_path(LIBMATHEVAL_INCLUDE_DIR matheval.h)
find_library(LIBMATHEVAL matheval)

add_library(vvtest SHARED
    TEval.cpp
)
target_link_libraries(vvtest ${LIBMATHEVAL})


