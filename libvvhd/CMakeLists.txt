cmake_minimum_required (VERSION 3.0)
project (libvvhd)

set(LIBVVHD_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/headers)
set(LIBVVHD_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/headers PARENT_SCOPE)
add_subdirectory(test)

# HDF5
find_package(HDF5 REQUIRED)
include_directories(${HDF5_INCLUDE_DIRS})

# LAPACK
if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
    find_package(LAPACK REQUIRED)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fopenmp")
elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Intel")
    find_library(MKL_LIBRARIES mkl_core)
    set(LAPACK_LIBRARIES "")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -qopenmp -mkl")
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -debug all")
    set(CMAKE_SHARED_LINKER_FLAGS "-mkl=parallel")
endif()

# MATHEVAL
find_path(LIBMATHEVAL_INCLUDE_DIR matheval.h)
find_library(LIBMATHEVAL matheval)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall")

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
set (LIBVVHD_SOURCES ${LIBVVHD_SOURCES} src/TEval.cpp)
set (LIBVVHD_SOURCES ${LIBVVHD_SOURCES} src/TBody.cpp)
set (LIBVVHD_SOURCES ${LIBVVHD_SOURCES} src/TSpace.cpp)
set (LIBVVHD_SOURCES ${LIBVVHD_SOURCES} src/TSortedTree.cpp)
set (LIBVVHD_SOURCES ${LIBVVHD_SOURCES} src/TMatrix.cpp)
set (LIBVVHD_SOURCES ${LIBVVHD_SOURCES} src/vvhdf.cpp)
set (LIBVVHD_SOURCES ${LIBVVHD_SOURCES} src/MStepdata.cpp)
set (LIBVVHD_SOURCES ${LIBVVHD_SOURCES} src/MConvectiveFast.cpp)
set (LIBVVHD_SOURCES ${LIBVVHD_SOURCES} src/MDiffusiveFast.cpp)
set (LIBVVHD_SOURCES ${LIBVVHD_SOURCES} src/MEpsFast.cpp)
set (LIBVVHD_SOURCES ${LIBVVHD_SOURCES} src/MFlowmove.cpp)

set (LIBVVHD_SOURCES ${LIBVVHD_SOURCES} src/XField.cpp)
set (LIBVVHD_SOURCES ${LIBVVHD_SOURCES} src/XIsoline.cpp)
set (LIBVVHD_SOURCES ${LIBVVHD_SOURCES} src/XVorticity.cpp)
set (LIBVVHD_SOURCES ${LIBVVHD_SOURCES} src/XStreamline.cpp)
add_library(vvhd SHARED ${LIBVVHD_SOURCES})
target_link_libraries(vvhd ${HDF5_C_LIBRARIES} ${LAPACK_LIBRARIES} ${LIBMATHEVAL})

install (TARGETS vvhd DESTINATION lib)
