cmake_minimum_required (VERSION 3.0)

include_directories(${LIBVVHD_INCLUDE_DIRS})
include_directories(${CMAKE_CURRENT_SOURCE_DIR})

ExternalProject_Add(googletest
    URL https://github.com/google/googletest/archive/v1.10.x.tar.gz
    CMAKE_ARGS
        -DCMAKE_INSTALL_PREFIX=<INSTALL_DIR>
)

ExternalProject_Get_Property(googletest install_dir)
include_directories(${install_dir}/include)

add_executable(libvvhd_test main.cpp)
add_dependencies(libvvhd_test googletest)
target_link_libraries(libvvhd_test
    vvhd
    ${install_dir}/lib/libgtest.a
    ${install_dir}/lib/libgtest_main.a
    -lpthread
)
set_target_properties(libvvhd_test PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/tests/
)

add_test(libvvhd_test ${CMAKE_BINARY_DIR}/tests/libvvhd_test)
