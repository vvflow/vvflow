cmake_minimum_required (VERSION 3.0)
project (vvplot3)






add_subdirectory(./../../libvvhd libvvhd)
include_directories(${LIBVVHD_INCLUDE_DIR})

add_executable(vvplot3
    vvplot.cpp
    # exear.cpp
    # map_vorticity.cpp
)

# find_package(LibArchive REQUIRED)
# include_directories(${LibArchive_INCLUDE_DIR})
find_library(LIBARCHIVE libarchive.a)
target_link_libraries(vvplot3 vvhd ${LIBARCHIVE})

# include_directories(${GFlags_INCLUDE_DIR})
# find_library(LIBGFLAGS libgflags_nothreads.a)
# target_link_libraries(vvplot ${LIBGFLAGS})

if(NOT NO_MANPAGES)
SET(MAN_NAMES vvplot.1)
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
add_custom_target(vvplot.man ALL DEPENDS ${MAN_FILES})
endif()

install (TARGETS vvplot3 DESTINATION bin)
install (FILES ${MAN_FILES} DESTINATION share/man/man1)
