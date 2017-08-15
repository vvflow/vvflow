cmake_minimum_required (VERSION 3.0)
project (vvscripts)

# HDF5
find_package(HDF5 REQUIRED)
include_directories(${HDF5_INCLUDE_DIRS})

add_subdirectory(./../../libvvhd libvvhd)
include_directories(${LIBVVHD_INCLUDE_DIR})

include_directories(source/)
set(LIBVVPLOT_SOURCES ${LIBVVPLOT_SOURCES} source/map_extract.cpp)
set(LIBVVPLOT_SOURCES ${LIBVVPLOT_SOURCES} source/map_vorticity.cpp)
set(LIBVVPLOT_SOURCES ${LIBVVPLOT_SOURCES} source/map_pressure.cpp)
set(LIBVVPLOT_SOURCES ${LIBVVPLOT_SOURCES} source/map_streamfunction.cpp)
set(LIBVVPLOT_SOURCES ${LIBVVPLOT_SOURCES} source/map_velocity.cpp)
set(LIBVVPLOT_SOURCES ${LIBVVPLOT_SOURCES} source/isoline.cpp)

add_library(vvplot SHARED ${LIBVVPLOT_SOURCES})
target_link_libraries(vvplot vvhd)
install(TARGETS
	vvplot
	DESTINATION lib
)

add_executable(libvvplot source/libvvplot_main.cpp)
target_link_libraries(libvvplot vvplot vvhd)
install(TARGETS
	libvvplot
	DESTINATION bin
)

install(PROGRAMS
    vvplot
    argparse_vvplot.py
    DESTINATION bin
)
