cmake_minimum_required (VERSION 3.0)
project (vvflow)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

add_subdirectory(utils/vvflow)
add_subdirectory(utils/vvxtract)
add_subdirectory(utils/vvcompose)
add_subdirectory(utils/scripts)

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
SET(CPACK_DEBIAN_PACKAGE_MAINTAINER "Yaroslav Dynnikov") #required
SET(CPACK_DEBIAN_PACKAGE_SECTION "science")
SET(CPACK_PACKAGE_CONTACT "yaroslav.dynnikov@gmail.com")

SET(CPACK_PACKAGE_DESCRIPTION "CFD simulation software")
SET(CPACK_PACKAGE_DESCRIPTION_SUMMARY
    "Viscous Vortex Domains (VVD) simulation software
    The VVD method is a mesh-free method of computational fluid dynamics
    for directly numerically solving 2D Navier-Stokes equations in Lagrange coordinates.
    This package provides tools for performing numerical experiment:
    composing a problem, solving it, processing results and doing visualization.")

INCLUDE(CPack)
