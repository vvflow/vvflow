cmake_minimum_required (VERSION 3.0)

#add_subdirectory(./../../libvvhd libvvhd)
#include_directories(${LIBVVHD_INCLUDE_DIR})

# # MATHEVAL
# find_path(LIBMATHEVAL_INCLUDE_DIR matheval.h)
# find_library(LIBMATHEVAL matheval)

# Locate GTest
find_package(GTest REQUIRED)
include_directories(${GTEST_INCLUDE_DIRS})

include_directories(../)
add_executable(test_vvtest
    test_TEval.cpp
)
target_link_libraries(test_vvtest vvtest ${GTEST_LIBRARIES} pthread)

if(NOT NO_SELFTEST)
add_custom_command(
    TARGET test_vvtest POST_BUILD
    COMMAND test_vvtest
)
endif()
