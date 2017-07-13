cmake_minimum_required (VERSION 3.0)
set (CMAKE_CXX_STANDARD 11)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/Modules/")
project (vvcompose2)

set(__bak_LIBRARY_SUFFIXES ${CMAKE_FIND_LIBRARY_SUFFIXES})
SET(CMAKE_FIND_LIBRARY_SUFFIXES .a)
find_package(Lua 5.2 EXACT REQUIRED)
find_library(LUA_LIBRARY liblua5.2.a)
SET(CMAKE_FIND_LIBRARY_SUFFIXES ${__bak_LIBRARY_SUFFIXES})
include_directories(${LUA_INCLUDE_DIR})

add_subdirectory(./../../libvvhd libvvhd)
include_directories(${LIBVVHD_INCLUDE_DIR})

add_executable(vvcompose2
    vvcompose2.cpp
    getset.cpp
    lua_tvec.cpp
    lua_tobj.cpp
    lua_tvec3d.cpp
    lua_shellscript.cpp
    lua_tbody.cpp
    lua_objlist.cpp
    lua_bodylist.cpp
    gen_cylinder.cpp
    gen_plate.cpp
)
target_link_libraries(vvcompose2 ${LIBVVHD_LIBRARIES} ${LUA_LIBRARY} dl)
add_custom_command(
    TARGET vvcompose2 POST_BUILD
    COMMAND vvcompose2 ${CMAKE_CURRENT_SOURCE_DIR}/test.lua
)

SET(MAN_NAMES vvcompose.1)
SET(MAN_FILES)
foreach(m IN LISTS MAN_NAMES)
    set(mf ${CMAKE_CURRENT_BINARY_DIR}/${m})
    set(ms ${CMAKE_CURRENT_SOURCE_DIR}/${m}.md)
    add_custom_command(
        OUTPUT ${mf}
        COMMAND ronn --roff --pipe ${ms} > ${mf}
        DEPENDS ${ms}
        WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
        COMMENT "Building manpage ${mf}"
        VERBATIM)
    list(APPEND MAN_FILES ${mf})
endforeach()
add_custom_target(man ALL DEPENDS ${MAN_FILES})

install (TARGETS vvcompose2 DESTINATION bin)
install (FILES ${MAN_FILES} DESTINATION share/man/man1)



