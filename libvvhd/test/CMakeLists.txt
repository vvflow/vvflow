cmake_minimum_required (VERSION 3.0)

include_directories(${LIBVVHD_INCLUDE_DIRS})
include_directories(${CMAKE_CURRENT_SOURCE_DIR})

# Locate GTest
find_package(GTest REQUIRED)
include_directories(${GTEST_INCLUDE_DIRS})

add_executable(libvvhd_test main.cpp)
target_link_libraries(libvvhd_test vvhd ${GTEST_LIBRARIES} pthread)
set_target_properties(libvvhd_test PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/tests/
)

add_test(libvvhd_test ${CMAKE_BINARY_DIR}/tests/libvvhd_test)
