cmake_minimum_required (VERSION 3.0)

include_directories(${LIBVVHD_INCLUDE_DIR})

add_executable(vvplot3
    vvplot.cpp
    optparse.cpp
    exear.cpp
    # map_vorticity.cpp
)

# find_package(LibArchive REQUIRED)
# include_directories(${LibArchive_INCLUDE_DIR})
find_library(LIBARCHIVE libarchive.a)
target_link_libraries(vvplot3 vvhd ${LIBARCHIVE})

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

install (TARGETS vvplot3 DESTINATION bin)
install (FILES ${MAN_ROFF} DESTINATION share/man/man1)
install (FILES ${MAN_HTML} DESTINATION share/doc/vvflow)
