cmake_minimum_required (VERSION 3.0)
project (libvvhd)

include(${CMAKE_SOURCE_DIR}/libvvhd/CPackDeps.cmake)

# HDF5
find_package(HDF5 REQUIRED)
add_dependency(${HDF5_C_LIBRARIES})
include_directories(${HDF5_INCLUDE_DIRS})

# LAPACK
if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
    find_package(LAPACK REQUIRED)
    add_dependency(${LAPACK_LIBRARIES})
    list(APPEND DEPENDENCIES "libgomp1")
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
find_library(MATHEVAL_LIBRARIES matheval)
add_dependency(${MATHEVAL_LIBRARIES})

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
message("-- Extracted git-rev: "  ${LIBVVHD_GITREV})
message("-- Extracted git-info: " ${LIBVVHD_GITINFO})
message("-- Extracted git-diff: " ${LIBVVHD_GITDIFF})
configure_file (
    "gitinfo.cpp.in"
    "${PROJECT_BINARY_DIR}/gitinfo.cpp"
  )

include_directories(headers/)
add_library(vvhd SHARED
    ${PROJECT_BINARY_DIR}/gitinfo.cpp
    src/TEval.cpp
    src/TBody.cpp
    src/TSpace.cpp
    src/TSpace_save_hdf.cpp
    src/TSpace_load_hdf.cpp
    src/TSpace_load_v13.cpp
    src/TSortedTree.cpp
    src/TMatrix.cpp
    src/vvhdf.cpp
    src/vvhdf_h5t.cpp
    src/MStepdata.cpp
    src/MConvectiveFast.cpp
    src/MDiffusiveFast.cpp
    src/MEpsilonFast.cpp
    src/MFlowmove.cpp
    src/XField.cpp
    src/XIsoline.cpp
    src/XPressure.cpp
    src/XVorticity.cpp
    src/XStreamfunction.cpp
    src/XVelocity.cpp
)
target_link_libraries(vvhd ${HDF5_C_LIBRARIES} ${LAPACK_LIBRARIES} ${MATHEVAL_LIBRARIES})

install (TARGETS vvhd DESTINATION lib)
set(DEPENDENCIES ${DEPENDENCIES} PARENT_SCOPE)
set(LIBVVHD_INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/headers ${HDF5_INCLUDE_DIRS})
set(LIBVVHD_INCLUDE_DIRS ${LIBVVHD_INCLUDE_DIRS} PARENT_SCOPE)
add_subdirectory(test)
