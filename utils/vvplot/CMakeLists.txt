cmake_minimum_required (VERSION 3.0)
project (vvplot)

include(${CMAKE_SOURCE_DIR}/libvvhd/CPackDeps.cmake)
include_directories(${LIBVVHD_INCLUDE_DIRS})

find_package(LibArchive REQUIRED)
add_dependency(${LibArchive_LIBRARY})
include_directories(${LibArchive_INCLUDE_DIR})

add_executable(vvplot
    vvplot.cpp
    optparse.cpp
    gnuplotter.cpp
    # map_vorticity.cpp
)
target_link_libraries(vvplot vvhd ${LibArchive_LIBRARIES})

if(NOT NO_MANPAGES)
set(MAN_MD ${CMAKE_CURRENT_SOURCE_DIR}/vvplot.1.md)
set(MAN_ROFF ${CMAKE_CURRENT_BINARY_DIR}/vvplot.1)
set(MAN_HTML ${CMAKE_CURRENT_BINARY_DIR}/vvplot.1.html)
add_custom_command(
    OUTPUT ${MAN_ROFF} ${MAN_HTML}
    COMMAND ronn --roff --pipe ${MAN_MD} > ${MAN_ROFF}
    COMMAND ronn --html --pipe ${MAN_MD} > ${MAN_HTML}
    DEPENDS ${MAN_MD}
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    COMMENT "Building manpage ${MAN_ROFF}"
    VERBATIM)
add_custom_target(vvplot.man ALL DEPENDS ${MAN_ROFF} ${MAN_HTML})
endif()

install (TARGETS vvplot DESTINATION bin)
install (FILES ${MAN_ROFF} DESTINATION share/man/man1)
install (FILES ${MAN_HTML} DESTINATION share/doc/vvflow)
set(DEPENDENCIES ${DEPENDENCIES} PARENT_SCOPE)
