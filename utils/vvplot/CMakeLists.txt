cmake_minimum_required (VERSION 3.0)
project (vvplot)

include(ExternalProject)
include_directories(${LIBVVHD_INCLUDE_DIRS})

ExternalProject_Add(libarchive
    URL https://www.libarchive.org/downloads/libarchive-3.4.3.tar.gz
    CONFIGURE_COMMAND <SOURCE_DIR>/configure
        --prefix=<INSTALL_DIR>
        --disable-shared
        --enable-static
        --with-pic

        --without-zlib
        --without-bz2lib
        --without-libb2
        --without-iconv
        --without-libiconv-prefix
        --without-lz4
        --without-zstd
        --without-lzma
        --without-cng
        --without-openssl
        --without-xml2
        --without-expat
)

ExternalProject_Get_Property(libarchive install_dir)
include_directories(${install_dir}/include)

add_executable(vvplot
    vvplot.cpp
    optparse.cpp
    gnuplotter.cpp
)
add_dependencies(vvplot libarchive)
target_link_libraries(vvplot
    vvhd
    ${install_dir}/lib/libarchive.a
)

if(NOT NO_MANPAGES)
set(MAN_MD ${CMAKE_CURRENT_SOURCE_DIR}/vvplot.1.md)
set(MAN_ROFF ${CMAKE_CURRENT_BINARY_DIR}/vvplot.1)
set(MAN_HTML ${CMAKE_CURRENT_BINARY_DIR}/vvplot.1.html)
add_custom_command(
    OUTPUT ${MAN_ROFF} ${MAN_HTML}
    COMMAND ronn --roff --pipe ${MAN_MD} > ${MAN_ROFF}
    COMMAND ronn --html --pipe ${MAN_MD} > ${MAN_HTML}
    DEPENDS ${MAN_MD}
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    COMMENT "Building manpage ${MAN_ROFF}"
    VERBATIM)
add_custom_target(vvplot.man ALL DEPENDS ${MAN_ROFF} ${MAN_HTML})
endif()

install (TARGETS vvplot DESTINATION bin)
install (FILES ${MAN_ROFF} DESTINATION share/man/man1)
install (FILES ${MAN_HTML} DESTINATION share/doc/vvflow)
set(DEPENDENCIES ${DEPENDENCIES} PARENT_SCOPE)
