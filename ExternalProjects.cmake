include(ExternalProject)

set(LUA_VERSION 5.2.4)
set(ZLIB_VERSION 1.2.11)
set(HDF5_VERSION 1.10.6)
set(LIBARCHIVE_VERSION 3.4.3)
set(CPPUNIT_VERSION 1.15.1)

string(CONCAT HDF5_URL
    "https://support.hdfgroup.org/ftp/HDF5/releases/"
    "hdf5-1.10/hdf5-${HDF5_VERSION}/src/hdf5-${HDF5_VERSION}.tar.gz"
)
string(CONCAT LIBARCHIVE_URL
    "https://www.libarchive.org/downloads/"
    "libarchive-${LIBARCHIVE_VERSION}.tar.gz"
)

#
# ZLIB
#
ExternalProject_Add(zlib
    URL https://vvflow.github.io/vvflow-deps/zlib-${ZLIB_VERSION}.tar.gz
        https://zlib.net/zlib-${ZLIB_VERSION}.tar.gz
    URL_MD5 1c9f62f0778697a09d36121ead88e08e
    CONFIGURE_COMMAND CFLAGS=-fPIC <SOURCE_DIR>/configure
        --prefix=<INSTALL_DIR>
        --static
    TEST_COMMAND ${CMAKE_MAKE_PROGRAM} check
)
ExternalProject_Get_Property(zlib install_dir)
if(IS_ABSOLUTE "${install_dir}")
    set(ZLIB_DIR ${install_dir})
    set(ZLIB_LIB_DIR ${install_dir}/lib)
    set(ZLIB_INCLUDE_DIRS ${install_dir}/include)
else()
    set(ZLIB_DIR ${CMAKE_CURRENT_BINARY_DIR}/${install_dir})
    set(ZLIB_LIB_DIR ${CMAKE_CURRENT_BINARY_DIR}/${install_dir}/lib)
    set(ZLIB_INCLUDE_DIRS ${CMAKE_CURRENT_BINARY_DIR}/${install_dir}/include)
endif()

# Create an INTERFACE library for ZLIB using link directories instead of file paths
add_library(zlib_external INTERFACE)
add_dependencies(zlib_external zlib)
target_link_directories(zlib_external INTERFACE ${ZLIB_LIB_DIR})
target_link_libraries(zlib_external INTERFACE z)
target_include_directories(zlib_external INTERFACE ${ZLIB_INCLUDE_DIRS})

#
# HDF5
#
ExternalProject_Add(hdf5
    URL https://vvflow.github.io/vvflow-deps/hdf5-${HDF5_VERSION}.tar.gz
        ${HDF5_URL}
    URL_MD5 37f3089e7487daf0890baf3d3328e54a
    DEPENDS zlib
    CONFIGURE_COMMAND <SOURCE_DIR>/configure
        --prefix=<INSTALL_DIR>
        --disable-shared
        --disable-cxx
        --disable-hl
        --disable-tests
        --disable-tools
        --with-zlib=${ZLIB_DIR}
        --enable-static
        --with-pic
    STEP_TARGETS download
)
ExternalProject_Get_Property(hdf5 install_dir)
if(IS_ABSOLUTE "${install_dir}")
    set(HDF5_INCLUDE_DIRS ${install_dir}/include)
    set(HDF5_LIB_DIR ${install_dir}/lib)
else()
    set(HDF5_INCLUDE_DIRS ${CMAKE_CURRENT_BINARY_DIR}/${install_dir}/include)
    set(HDF5_LIB_DIR ${CMAKE_CURRENT_BINARY_DIR}/${install_dir}/lib)
endif()

# Create an INTERFACE library for HDF5 using link directories instead of file paths
add_library(hdf5_external INTERFACE)
add_dependencies(hdf5_external hdf5)
target_link_directories(hdf5_external INTERFACE ${HDF5_LIB_DIR})
target_link_libraries(hdf5_external INTERFACE hdf5 zlib_external)
target_include_directories(hdf5_external INTERFACE ${HDF5_INCLUDE_DIRS})

#
# Lua
#
ExternalProject_Add(lua
    URL https://vvflow.github.io/vvflow-deps/lua-${LUA_VERSION}.tar.gz
        https://www.lua.org/ftp/lua-${LUA_VERSION}.tar.gz
    URL_MD5 913fdb32207046b273fdb17aad70be13
    CONFIGURE_COMMAND ""
    BUILD_COMMAND make -C <SOURCE_DIR>/src liblua.a MYCFLAGS=-fPIC
    INSTALL_COMMAND ""
)

ExternalProject_Get_Property(lua source_dir)
if(IS_ABSOLUTE "${source_dir}")
    set(LUA_INCLUDE_DIRS ${source_dir}/src)
    set(LUA_LIB_DIR ${source_dir}/src)
else()
    set(LUA_INCLUDE_DIRS ${CMAKE_CURRENT_BINARY_DIR}/${source_dir}/src)
    set(LUA_LIB_DIR ${CMAKE_CURRENT_BINARY_DIR}/${source_dir}/src)
endif()

# Create an INTERFACE library for Lua using link directories instead of file paths
add_library(lua_external INTERFACE)
add_dependencies(lua_external lua)
target_link_directories(lua_external INTERFACE ${LUA_LIB_DIR})
target_link_libraries(lua_external INTERFACE lua)
target_include_directories(lua_external INTERFACE ${LUA_INCLUDE_DIRS})

#
# LibArchive
#
ExternalProject_Add(libarchive
    URL https://vvflow.github.io/vvflow-deps/libarchive-${LIBARCHIVE_VERSION}.tar.gz
        ${LIBARCHIVE_URL}
    URL_MD5 2c5f01b65e74c5a5a6ce45cc01647a53
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
if(IS_ABSOLUTE "${install_dir}")
    set(LIBARCHIVE_INCLUDE_DIRS ${install_dir}/include)
    set(LIBARCHIVE_LIB_DIR ${install_dir}/lib)
else()
    set(LIBARCHIVE_INCLUDE_DIRS ${CMAKE_CURRENT_BINARY_DIR}/${install_dir}/include)
    set(LIBARCHIVE_LIB_DIR ${CMAKE_CURRENT_BINARY_DIR}/${install_dir}/lib)
endif()

# Create an INTERFACE library for LibArchive using link directories instead of file paths
add_library(libarchive_external INTERFACE)
add_dependencies(libarchive_external libarchive)
target_link_directories(libarchive_external INTERFACE ${LIBARCHIVE_LIB_DIR})
target_link_libraries(libarchive_external INTERFACE archive)
target_include_directories(libarchive_external INTERFACE ${LIBARCHIVE_INCLUDE_DIRS})

#
# CppUnit
#
ExternalProject_Add(cppunit
    URL https://vvflow.github.io/vvflow-deps/cppunit-${CPPUNIT_VERSION}.tar.gz
        http://dev-www.libreoffice.org/src/cppunit-${CPPUNIT_VERSION}.tar.gz
    URL_MD5 "9dc669e6145cadd9674873e24943e6dd"
    CONFIGURE_COMMAND <SOURCE_DIR>/configure
        --prefix=<INSTALL_DIR>
        --disable-doxygen
        --disable-docs
        # shared is enough for testing
        --enable-shared
        --disable-static
)
ExternalProject_Get_Property(cppunit install_dir)
if(IS_ABSOLUTE "${install_dir}")
    set(CPPUNIT_INCLUDE_DIRS ${install_dir}/include)
    set(CPPUNIT_LIB_DIR ${install_dir}/lib)
else()
    set(CPPUNIT_INCLUDE_DIRS ${CMAKE_CURRENT_BINARY_DIR}/${install_dir}/include)
    set(CPPUNIT_LIB_DIR ${CMAKE_CURRENT_BINARY_DIR}/${install_dir}/lib)
endif()

# Create an INTERFACE library for CppUnit using link directories instead of file paths
add_library(cppunit_external INTERFACE)
add_dependencies(cppunit_external cppunit)
target_link_directories(cppunit_external INTERFACE ${CPPUNIT_LIB_DIR})
target_link_libraries(cppunit_external INTERFACE cppunit)
target_include_directories(cppunit_external INTERFACE ${CPPUNIT_INCLUDE_DIRS})
