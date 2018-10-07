cmake_minimum_required (VERSION 3.0)
project (vvflow)

include_directories(${LIBVVHD_INCLUDE_DIRS})

add_executable(vvflow vvflow.cpp)
target_link_libraries(vvflow vvhd)

if(NOT NO_MANPAGES)
set(MAN_MD ${CMAKE_CURRENT_SOURCE_DIR}/vvflow.1.md)
set(MAN_ROFF ${CMAKE_CURRENT_BINARY_DIR}/vvflow.1)
set(MAN_HTML ${CMAKE_CURRENT_BINARY_DIR}/vvflow.1.html)
add_custom_command(
    OUTPUT ${MAN_ROFF} ${MAN_HTML}
    COMMAND ronn --roff --pipe ${MAN_MD} > ${MAN_ROFF}
    COMMAND ronn --html --pipe ${MAN_MD} > ${MAN_HTML}
    DEPENDS ${MAN_MD}
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    COMMENT "Building manpage ${MAN_ROFF}"
    VERBATIM)
add_custom_target(vvflow.man ALL DEPENDS ${MAN_ROFF} ${MAN_HTML})
endif()

install (TARGETS vvflow DESTINATION bin)
install (FILES ${MAN_ROFF} DESTINATION share/man/man1)
install (FILES ${MAN_HTML} DESTINATION share/doc/vvflow)
