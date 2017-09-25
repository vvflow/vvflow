cmake_minimum_required (VERSION 3.0)

include_directories(${LIBVVHD_INCLUDE_DIR})

# Locate GTest
find_package(GTest REQUIRED)
include_directories(${GTEST_INCLUDE_DIRS})

add_executable(libvvhd.test
	main.cpp
)
target_link_libraries(libvvhd.test vvhd ${GTEST_LIBRARIES} pthread)

if(NOT NO_SELFTEST)
add_custom_command(
    TARGET libvvhd.test POST_BUILD
    COMMAND libvvhd.test
)
endif()
