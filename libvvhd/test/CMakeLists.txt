cmake_minimum_required (VERSION 3.0)

include_directories(
	${LIBVVHD_INCLUDE_DIRS}
	${GTEST_INCLUDE_DIRS}
	# ${CMAKE_CURRENT_SOURCE_DIR}
)

add_executable(libvvhd_test main.cpp)
add_dependencies(libvvhd_test googletest)
target_link_libraries(libvvhd_test vvhd ${GTEST_LIBRARIES})
set_target_properties(libvvhd_test PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/tests/
)

add_test(libvvhd_test ${CMAKE_BINARY_DIR}/tests/libvvhd_test)
