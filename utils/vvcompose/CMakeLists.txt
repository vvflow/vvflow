cmake_minimum_required (VERSION 3.0)
project (vvcompose)

find_library(LUA_LIBRARY lua5.2)
find_path(LUA_INCLUDE_DIR lua.hpp
    PATH_SUFFIXES lua5.2
)
message("LUA_LIBRARY: " ${LUA_LIBRARY})
message("LUA_INCLUDE_DIR: " ${LUA_INCLUDE_DIR})
include_directories(${LUA_INCLUDE_DIR})

add_subdirectory(./../../libvvhd libvvhd)
include_directories(${LIBVVHD_INCLUDE_DIR})

add_executable(vvcompose
    vvcompose.cpp
    getset.cpp
    lua_space.cpp
    lua_tvec.cpp
    lua_tobj.cpp
    lua_tvec3d.cpp
    lua_shellscript.cpp
    lua_tbody.cpp
    lua_objlist.cpp
    lua_bodylist.cpp
    gen_body.cpp
    gen_cylinder.cpp
    gen_plate.cpp
    gen_gis.cpp
    linenoise.c
)
target_link_libraries(vvcompose vvhd ${LUA_LIBRARY} dl)

if(NOT NO_SELFTEST)
add_custom_command(
    TARGET vvcompose POST_BUILD
    COMMAND vvcompose ${CMAKE_CURRENT_SOURCE_DIR}/test.lua
)
endif()

if(NOT NO_MANPAGES)
SET(MAN_NAMES vvcompose.1)
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
add_custom_target(vvcompose.man ALL DEPENDS ${MAN_FILES})
endif()

install (TARGETS vvcompose DESTINATION bin)
install (FILES ${MAN_FILES} DESTINATION share/man/man1)
