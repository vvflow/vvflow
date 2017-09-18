cmake_minimum_required (VERSION 3.0)
project (vvflow)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

add_subdirectory(utils/vvflow)
add_subdirectory(utils/vvplot)
add_subdirectory(utils/vvplot3)
add_subdirectory(utils/vvxtract)
add_subdirectory(utils/vvcompose)
add_subdirectory(utils/scripts)

if(NOT NO_MANPAGES)
set(MAN_MD   ${CMAKE_CURRENT_SOURCE_DIR}/README.md)
set(MAN_HTML ${CMAKE_CURRENT_BINARY_DIR}/README.html)
add_custom_command(
    OUTPUT ${MAN_HTML}
    COMMAND grip --export ${MAN_MD} ${MAN_HTML}
    DEPENDS ${MAN_MD}
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    COMMENT "Building ${MAN_HTML}"
    VERBATIM)
add_custom_target(readme.man ALL DEPENDS ${MAN_HTML})
endif()
install (FILES ${MAN_HTML} DESTINATION share/doc/vvflow)

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
SET(CPACK_DEBIAN_PACKAGE_DEPENDS "liblua5.2-0, liblapack3, libmatheval1, libgomp1")
SET(CPACK_DEBIAN_PACKAGE_MAINTAINER "Yaroslav Dynnikov") #required
SET(CPACK_DEBIAN_PACKAGE_SECTION "science")
SET(CPACK_PACKAGE_CONTACT "yaroslav.dynnikov@gmail.com")

# find_path(HDF5_PATH libhdf5.so)
find_package(HDF5 REQUIRED)
execute_process(
    COMMAND readlink -f ${HDF5_LIBRARY_DIRS}/libhdf5.so
    OUTPUT_VARIABLE HDF5_LIBRARY
    OUTPUT_STRIP_TRAILING_WHITESPACE)
execute_process(
    COMMAND dpkg -S ${HDF5_LIBRARY}
    COMMAND cut -d: -f1
    OUTPUT_VARIABLE HDF5_PACKAGE
    OUTPUT_STRIP_TRAILING_WHITESPACE)
SET(CPACK_DEBIAN_PACKAGE_DEPENDS "${CPACK_DEBIAN_PACKAGE_DEPENDS}, ${HDF5_PACKAGE}")

SET(CPACK_PACKAGE_DESCRIPTION "CFD simulation software")
SET(CPACK_PACKAGE_DESCRIPTION_SUMMARY
    "Viscous Vortex Domains (VVD) simulation software
    The VVD method is a mesh-free method of computational fluid dynamics
    for directly numerically solving 2D Navier-Stokes equations in Lagrange coordinates.
    This package provides tools for performing numerical experiment:
    composing a problem, solving it, processing results and doing visualization.")

INCLUDE(CPack)
