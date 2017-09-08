cmake_minimum_required (VERSION 3.0)
# set (CMAKE_CXX_STANDARD 11)
project (vvxtract)

# HDF5
find_package(HDF5 REQUIRED)
include_directories(${HDF5_INCLUDE_DIRS})

add_subdirectory(./../../libvvhd libvvhd)
include_directories(${LIBVVHD_INCLUDE_DIR})

add_executable(vvxtract vvxtract.cpp)
target_link_libraries(vvxtract vvhd)

if(NOT NO_MANPAGES)
SET(MAN_NAMES vvxtract.1)
SET(MAN_FILES)
foreach(m IN LISTS MAN_NAMES)
    set(mf ${CMAKE_CURRENT_BINARY_DIR}/${m})
    set(ms ${CMAKE_CURRENT_SOURCE_DIR}/${m}.md)
    add_custom_command(
        OUTPUT ${mf}
        COMMAND ronn --roff --pipe ${ms} > ${mf}
        COMMAND ronn --html --pipe ${ms} > ${mf}.html
        DEPENDS ${ms}
        WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
        COMMENT "Building manpage ${mf}"
        VERBATIM)
    list(APPEND MAN_FILES ${mf})
endforeach()
add_custom_target(vvxtract.man ALL DEPENDS ${MAN_FILES})
endif()

install (TARGETS vvxtract DESTINATION bin)
install (FILES ${MAN_FILES} DESTINATION share/man/man1)
