cmake_minimum_required (VERSION 3.0)
project (vvcompose)

include(${CMAKE_SOURCE_DIR}/libvvhd/CPackDeps.cmake)
include_directories(${LIBVVHD_INCLUDE_DIRS})

find_path(LUA_INCLUDE_DIR lua.hpp PATH_SUFFIXES lua5.2)
find_library(LUA_LIBRARY lua5.2)
message("-- Found liblua5.2: " ${LUA_LIBRARY})
add_dependency(${LUA_LIBRARY})
include_directories(${LUA_INCLUDE_DIR})

add_executable(vvcompose
    vvcompose.cpp
    getset.cpp
    lua_space.cpp
    lua_tvec.cpp
    lua_tobj.cpp
    lua_tvec3d.cpp
    lua_teval.cpp
    lua_tbody.cpp
    lua_objlist.cpp
    lua_bodylist.cpp
    gen_body.cpp
    gen_cylinder.cpp
    gen_ellipse.cpp
    gen_parallelogram.cpp
    gen_plate.cpp
    gen_gis.cpp
    gen_box.cpp
    linenoise.c
)
target_link_libraries(vvcompose vvhd ${LUA_LIBRARY} dl)

if(NOT NO_MANPAGES)
set(MAN_MD ${CMAKE_CURRENT_SOURCE_DIR}/vvcompose.1.md)
set(MAN_ROFF ${CMAKE_CURRENT_BINARY_DIR}/vvcompose.1)
set(MAN_HTML ${CMAKE_CURRENT_BINARY_DIR}/vvcompose.1.html)
add_custom_command(
    OUTPUT ${MAN_ROFF} ${MAN_HTML}
    COMMAND ronn --roff --pipe ${MAN_MD} > ${MAN_ROFF}
    COMMAND ronn --html --pipe ${MAN_MD} > ${MAN_HTML}
    DEPENDS ${MAN_MD}
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    COMMENT "Building manpage ${MAN_ROFF}"
    VERBATIM)
add_custom_target(vvcompose.man ALL DEPENDS ${MAN_ROFF} ${MAN_HTML})
endif()

install (TARGETS vvcompose DESTINATION bin)
install (FILES ${MAN_ROFF} DESTINATION share/man/man1)
install (FILES ${MAN_HTML} DESTINATION share/doc/vvflow)
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/vvcompose_test.lua
    DESTINATION ${CMAKE_BINARY_DIR}/tests/
    USE_SOURCE_PERMISSIONS
)
add_test(vvcompose_test vvcompose ${CMAKE_BINARY_DIR}/tests/vvcompose_test.lua)
set(DEPENDENCIES ${DEPENDENCIES} PARENT_SCOPE)
