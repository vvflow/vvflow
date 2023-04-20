cmake_minimum_required (VERSION 3.0)
project (libvvhd)

find_package(LAPACK REQUIRED)
add_cpack_dependency(${LAPACK_LIBRARIES})
add_cpack_dependency("libgomp1")

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

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -fopenmp")

include_directories(
    headers/
    ${LUA_INCLUDE_DIRS}
    ${HDF5_INCLUDE_DIRS}
)
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
add_dependencies(vvhd lua hdf5)

find_library(CBLAS cblas
    HINTS ${PROJECT_BINARY_DIR}/../CBLAS/lib
    REQUIRED
)

target_link_libraries(vvhd
    ${CBLAS}
    ${LAPACK_LIBRARIES}
    ${HDF5_LIBRARIES}
    ${LUA_LIBRARIES}
    dl
)

install (TARGETS vvhd DESTINATION lib)
set(LIBVVHD_INCLUDE_DIRS
    ${CMAKE_CURRENT_SOURCE_DIR}/headers
    ${HDF5_INCLUDE_DIRS}
    ${LUA_INCLUDE_DIRS}
)
set(LIBVVHD_INCLUDE_DIRS ${LIBVVHD_INCLUDE_DIRS} PARENT_SCOPE)
add_subdirectory(test)
