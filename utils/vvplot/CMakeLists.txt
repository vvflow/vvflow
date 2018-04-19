cmake_minimum_required (VERSION 3.0)
project (vvscripts)

include_directories(${LIBVVHD_INCLUDE_DIRS})
include_directories(source/)
add_library(vvplot SHARED
    source/map_extract.cpp
    source/map_vorticity.cpp
    source/map_pressure.cpp
    source/map_streamfunction.cpp
    source/map_velocity.cpp
    source/isoline.cpp
)
target_link_libraries(vvplot vvhd)

add_executable(libvvplot source/libvvplot_main.cpp)
target_link_libraries(libvvplot vvplot vvhd)
set_target_properties(libvvplot PROPERTIES INSTALL_RPATH "$ORIGIN")

install(TARGETS
    vvplot
    libvvplot
    DESTINATION bin
)

install(PROGRAMS
    vvplot
    argparse_vvplot.py
    DESTINATION bin
)
