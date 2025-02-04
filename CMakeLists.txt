cmake_minimum_required (VERSION 3.0)
project (vvflow)

set(CPACK_DEPENDENCIES "" CACHE INTERNAL "")
function(add_cpack_dependency lib_list)
    list(GET lib_list 0 lib)
    message(STATUS "Adding CPack dependency for ${lib}")

    if(EXISTS ${lib})
        execute_process(
            COMMAND readlink -f "${lib}"
            OUTPUT_VARIABLE lib_path
            OUTPUT_STRIP_TRAILING_WHITESPACE)
        execute_process(
            COMMAND dpkg -S "${lib_path}"
            COMMAND cut -d: -f1
            OUTPUT_VARIABLE lib
            OUTPUT_STRIP_TRAILING_WHITESPACE)
    endif()

    set(_CPACK_DEPENDENCIES ${CPACK_DEPENDENCIES})
    list(APPEND _CPACK_DEPENDENCIES "${lib}")
    set(CPACK_DEPENDENCIES ${_CPACK_DEPENDENCIES}
        CACHE INTERNAL "Resulting DEB package dependencies"
    )
endfunction(add_cpack_dependency)

include(ExternalProjects.cmake)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -Weffc++")
enable_testing()

find_program(GRIP grip DOC "Render markdown files into html")
find_program(RONN ronn DOC "Build manuals from markdown")
if(GRIP AND RONN)
    set(WITH_MANPAGES TRUE CACHE BOOL "Build manpages")
else()
    set(WITH_MANPAGES FALSE CACHE BOOL "Build manpages")
endif()

if(WITH_MANPAGES)
    if(NOT GRIP)
        message(FATAL_ERROR "grip program not found")
    elseif(NOT RONN)
        message(FATAL_ERROR "ronn program not found")
    endif()
endif()

add_subdirectory(libvvhd)
add_subdirectory(utils/vvflow)
add_subdirectory(utils/vvplot)
add_subdirectory(utils/vvxtract)
add_subdirectory(utils/scripts)
add_subdirectory(pytest)

install(DIRECTORY example
    DESTINATION share/doc/vvflow
)

if(WITH_MANPAGES)
set(MAN_MD   ${CMAKE_CURRENT_SOURCE_DIR}/README.md)
set(MAN_HTML ${CMAKE_CURRENT_BINARY_DIR}/README.html)
add_custom_command(
    OUTPUT ${MAN_HTML}
    COMMAND ${GRIP} --export ${MAN_MD} ${MAN_HTML}
    DEPENDS ${MAN_MD}
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    COMMENT "Building ${MAN_HTML}"
    VERBATIM)
add_custom_target(readme.man ALL DEPENDS ${MAN_HTML})
endif()
install(FILES ${MAN_HTML} DESTINATION share/doc/vvflow)
install(DIRECTORY readme-pics DESTINATION share/doc/vvflow)

SET(CPACK_GENERATOR "DEB")
execute_process(
    COMMAND git describe --tags --always
    COMMAND cut -c2-
    COMMAND cut -d. -f1
    WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
    OUTPUT_VARIABLE MAJOR_VERSION
    OUTPUT_STRIP_TRAILING_WHITESPACE)
execute_process(
    COMMAND git describe --tags --always
    COMMAND cut -c2-
    COMMAND cut -d. -f2
    WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
    OUTPUT_VARIABLE MINOR_VERSION
    OUTPUT_STRIP_TRAILING_WHITESPACE)
execute_process(
    COMMAND git describe --tags --always
    COMMAND cut -c2-
    COMMAND cut -d. -f3
    WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
    OUTPUT_VARIABLE PATCH_VERSION
    OUTPUT_STRIP_TRAILING_WHITESPACE)
SET(CPACK_PACKAGE_VERSION_MAJOR "${MAJOR_VERSION}")
SET(CPACK_PACKAGE_VERSION_MINOR "${MINOR_VERSION}")
SET(CPACK_PACKAGE_VERSION_PATCH "${PATCH_VERSION}")
SET(CPACK_DEBIAN_PACKAGE_MAINTAINER "Yaroslav Dynnikov <yaroslav.dynnikov@gmail.com>") #mandatory
SET(CPACK_DEBIAN_PACKAGE_SECTION "science")
SET(CPACK_PACKAGE_DESCRIPTION
"CFD software for performing flow simulations with Viscous Vortex Domains (VVD)
method. The VVD method is a mesh-free method of computational fluid dynamics for
directly numerically solving 2D Navier-Stokes equations in Lagrange coordinates.
This package provides tools for performing numerical experiment: composing a
problem, solving it, processing results and doing visualization.")
if(NOT CPACK_OUTPUT_FILE_PREFIX)
    SET(CPACK_OUTPUT_FILE_PREFIX deb)
endif()
string(REPLACE ";" ", " CPACK_DEBIAN_PACKAGE_DEPENDS "${CPACK_DEPENDENCIES}")
message("-- Generating dpkg deps: " "${CPACK_DEBIAN_PACKAGE_DEPENDS}")
message("-- Will put deb package to: " "${CPACK_OUTPUT_FILE_PREFIX}")

INCLUDE(CPack)
