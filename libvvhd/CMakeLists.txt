cmake_minimum_required (VERSION 3.0)
set (CMAKE_CXX_STANDARD 11)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/Modules/")
project (libvvhd)

set(LIBVVHD_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/headers)
set(LIBVVHD_INCLUDE_DIR ${LIBVVHD_INCLUDE_DIR} PARENT_SCOPE)

set(HDF5_USE_STATIC_LIBRARIES 1)
find_package(HDF5 REQUIRED)
include_directories(${HDF5_INCLUDE_DIRS})

set (LIBVVHD_LIBRARIES "")
set (LIBVVHD_LIBRARIES ${LIBVVHD_LIBRARIES} vvhd)
set (LIBVVHD_LIBRARIES ${LIBVVHD_LIBRARIES} ${HDF5_C_LIBRARIES} aec)
set (LIBVVHD_LIBRARIES ${LIBVVHD_LIBRARIES} lapack blas gfortran quadmath)
set (LIBVVHD_LIBRARIES ${LIBVVHD_LIBRARIES} matheval fl)
set (LIBVVHD_LIBRARIES ${LIBVVHD_LIBRARIES} PARENT_SCOPE)

if(TARGET vvhd)

# message("libvvhd is already defined")

else()

include_directories(headers/)

set (LIBVVHD_SOURCES ${LIBVVHD_SOURCES} ../gitinfo.cpp)
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
