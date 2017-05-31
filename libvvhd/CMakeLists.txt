cmake_minimum_required (VERSION 3.0)
set (CMAKE_CXX_STANDARD 11)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/Modules/")
project (libvvhd)

set(LIBVVHD_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/headers)
set(LIBVVHD_INCLUDE_DIR ${LIBVVHD_INCLUDE_DIR} PARENT_SCOPE)

set (LIBVVHD_LIBRARIES "")
set (LIBVVHD_LIBRARIES ${LIBVVHD_LIBRARIES} vvhd)

# HDF5
set(HDF5_USE_STATIC_LIBRARIES 1)
find_package(HDF5 REQUIRED)
include_directories(${HDF5_INCLUDE_DIRS})
message("HDF5_C_LIBRARIES: " ${HDF5_C_LIBRARIES})
find_library(LIBPTHREAD pthread)
find_library(LIBDL      dl)
find_library(LIBAEC     libaec.a)
find_library(LIBSZ      libsz.a)
find_library(LIBZ       libz.a)
find_library(LIBM       libm.a)
set (LIBVVHD_LIBRARIES ${LIBVVHD_LIBRARIES}
    ${HDF5_hdf5_LIBRARY_RELEASE} ${LIBPTHREAD} ${LIBDL} # HDF5 is static, but pthread and dl are linked dynamically
    ${LIBSZ} ${LIBZ} ${LIBM} ${LIBAEC} # theese are linked statically
    )

# LAPACK
# set(BLA_STATIC 1)
find_package(LAPACK REQUIRED)
find_library(LIBLAPACK   liblapack.a)
find_library(LIBBLAS     libblas.a)
set (LIBVVHD_LIBRARIES ${LIBVVHD_LIBRARIES}
    ${LIBLAPACK} ${LIBBLAS} libgfortran.a libquadmath.a
    )

# MATHEVAL
find_library(LIBMATHEVAL libmatheval.a)
find_library(LIBFL       libfl.a)
set (LIBVVHD_LIBRARIES ${LIBVVHD_LIBRARIES}
    ${LIBMATHEVAL} ${LIBFL}
    )

set (LIBVVHD_LIBRARIES ${LIBVVHD_LIBRARIES} PARENT_SCOPE)

if(TARGET vvhd)

# message("libvvhd is already defined")

else()

execute_process(
    COMMAND git rev-parse HEAD
    WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
    OUTPUT_VARIABLE LIBVVHD_GITREV
    OUTPUT_STRIP_TRAILING_WHITESPACE)
execute_process(
    COMMAND git describe --tags --always
    WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
    OUTPUT_VARIABLE LIBVVHD_GITINFO
    OUTPUT_STRIP_TRAILING_WHITESPACE)
execute_process(
    COMMAND git diff --name-only
    COMMAND tr "\n" " "
    WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
    OUTPUT_VARIABLE LIBVVHD_GITDIFF
    OUTPUT_STRIP_TRAILING_WHITESPACE)
message("PROJECT_SOURCE_DIR: " ${PROJECT_SOURCE_DIR})
message("LIBVVHD_GITREV: "  ${LIBVVHD_GITREV})
message("LIBVVHD_GITINFO: " ${LIBVVHD_GITINFO})
message("LIBVVHD_GITDIFF: " ${LIBVVHD_GITDIFF})
configure_file (
    "gitinfo.cpp.in"
    "${PROJECT_BINARY_DIR}/gitinfo.cpp"
  )
include_directories(headers/)

set (LIBVVHD_SOURCES ${LIBVVHD_SOURCES} ${PROJECT_BINARY_DIR}/gitinfo.cpp)
set (LIBVVHD_SOURCES ${LIBVVHD_SOURCES} src/core/body.cpp)
set (LIBVVHD_SOURCES ${LIBVVHD_SOURCES} src/core/shellscript.cpp)
set (LIBVVHD_SOURCES ${LIBVVHD_SOURCES} src/core/sorted_tree.cpp)
set (LIBVVHD_SOURCES ${LIBVVHD_SOURCES} src/core/space.cpp)
# set (LIBVVHD_SOURCES ${LIBVVHD_SOURCES} src/core/space_hdf.cpp)
set (LIBVVHD_SOURCES ${LIBVVHD_SOURCES} src/core/stepdata.cpp)
set (LIBVVHD_SOURCES ${LIBVVHD_SOURCES} src/modules/convectivefast.cpp)
set (LIBVVHD_SOURCES ${LIBVVHD_SOURCES} src/modules/diffusivefast.cpp)
set (LIBVVHD_SOURCES ${LIBVVHD_SOURCES} src/modules/epsfast.cpp)
set (LIBVVHD_SOURCES ${LIBVVHD_SOURCES} src/modules/flowmove.cpp)
set (LIBVVHD_SOURCES ${LIBVVHD_SOURCES} src/modules/matrix.cpp)
add_library(vvhd STATIC ${LIBVVHD_SOURCES})

endif()
