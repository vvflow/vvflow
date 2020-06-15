include(ExternalProject)

set(LUA_VERSION 5.2.4)
set(ZLIB_VERSION 1.2.11)
set(HDF5_VERSION 1.10.6)
set(LIBARCHIVE_VERSION 3.4.3)

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
    URL https://zlib.net/zlib-${ZLIB_VERSION}.tar.gz
    CONFIGURE_COMMAND CFLAGS=-fPIC <SOURCE_DIR>/configure
        --prefix=<INSTALL_DIR>
        --static
    TEST_COMMAND ${CMAKE_MAKE_PROGRAM} check
)
ExternalProject_Get_Property(zlib install_dir)
set(ZLIB_DIR ${install_dir})
set(ZLIB_LIBRARIES ${install_dir}/lib/libz.a)
set(ZLIB_INCLUDE_DIRS ${install_dir}/include)

#
# HDF5
#
ExternalProject_Add(hdf5
    # URL https://support.hdfgroup.org/ftp/HDF5/releases/hdf5-1.10/hdf5-1.10.6/src/CMake-hdf5-1.10.6.tar.gz
    URL ${HDF5_URL}
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
    # CMAKE_ARGS
    #     -DCMAKE_INSTALL_PREFIX=<INSTALL_DIR>
    #     -DCMAKE_PREFIX_PATH=${ZLIB_PREFIX_PATH}
    #     -DBUILD_SHARED_LIBS=FALSE
    #     -DBUILD_TESTING=FALSE
    #     -DHDF5_BUILD_EXAMPLES=FALSE
    #     -DHDF5_BUILD_CPP_LIB=FALSE
    #     -DHDF5_BUILD_TOOLS=FALSE
    #     -DHDF5_BUILD_HL_LIB=FALSE
    #     -DHDF5_DISABLE_COMPILER_WARNINGS=TRUE
    #     -DHDF5_ENABLE_Z_LIB_SUPPORT=TRUE
    STEP_TARGETS download
)
ExternalProject_Get_Property(hdf5 install_dir)
set(HDF5_INCLUDE_DIRS ${install_dir}/include)
set(HDF5_LIBRARIES ${install_dir}/lib/libhdf5.a ${ZLIB_LIBRARIES})

#
# Lua
#
ExternalProject_Add(lua
    URL https://www.lua.org/ftp/lua-${LUA_VERSION}.tar.gz
    CONFIGURE_COMMAND ""
    BUILD_COMMAND make -C <SOURCE_DIR>/src liblua.a MYCFLAGS=-fPIC
    INSTALL_COMMAND ""
)

ExternalProject_Get_Property(lua source_dir)
set(LUA_INCLUDE_DIRS ${source_dir}/src)
set(LUA_LIBRARIES ${source_dir}/src/liblua.a)

#
# GoogleTest
#
ExternalProject_Add(googletest
    URL https://github.com/google/googletest/archive/v1.10.x.tar.gz
    CMAKE_ARGS
        -DCMAKE_INSTALL_PREFIX=<INSTALL_DIR>
)

ExternalProject_Get_Property(googletest install_dir)
set(GTEST_INCLUDE_DIRS ${install_dir}/include)
set(GTEST_LIBRARIES
    ${install_dir}/lib/libgtest.a
    ${install_dir}/lib/libgtest_main.a
    -lpthread
)

#
# LibArchive
#
ExternalProject_Add(libarchive
    URL ${LIBARCHIVE_URL}
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
set(LIBARCHIVE_INCLUDE_DIRS ${install_dir}/include)
set(LIBARCHIVE_LIBRARIES ${install_dir}/lib/libarchive.a)
