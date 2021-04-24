cmake_minimum_required (VERSION 3.0)
project (vvcompose)

include_directories(${LIBVVHD_INCLUDE_DIRS})

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
    gen_savonius.cpp
    linenoise.c
)
target_link_libraries(vvcompose vvhd dl)

if(WITH_MANPAGES)
set(MAN_MD ${CMAKE_CURRENT_SOURCE_DIR}/vvcompose.1.md)
set(MAN_ROFF ${CMAKE_CURRENT_BINARY_DIR}/vvcompose.1)
set(MAN_HTML ${CMAKE_CURRENT_BINARY_DIR}/vvcompose.1.html)
add_custom_command(
    OUTPUT ${MAN_ROFF} ${MAN_HTML}
    COMMAND ${RONN} --roff --pipe ${MAN_MD} > ${MAN_ROFF}
    COMMAND ${RONN} --html --pipe ${MAN_MD} > ${MAN_HTML}
    DEPENDS ${MAN_MD}
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    COMMENT "Building manpage ${MAN_ROFF}"
    VERBATIM)
add_custom_target(vvcompose.man ALL DEPENDS ${MAN_ROFF} ${MAN_HTML})
endif()

install (TARGETS vvcompose DESTINATION bin)
install (FILES ${MAN_ROFF} DESTINATION share/man/man1)
install (FILES ${MAN_HTML} DESTINATION share/doc/vvflow)
add_custom_command(
    OUTPUT ${CMAKE_BINARY_DIR}/tests/vvcompose_test.lua
    COMMAND ${CMAKE_COMMAND} -E copy
            ${CMAKE_CURRENT_SOURCE_DIR}/vvcompose_test.lua
            ${CMAKE_BINARY_DIR}/tests/vvcompose_test.lua
    DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/vvcompose_test.lua)
add_custom_target(vvcompose_test.lua ALL
    DEPENDS ${CMAKE_BINARY_DIR}/tests/vvcompose_test.lua)

add_test(vvcompose_test vvcompose ${CMAKE_BINARY_DIR}/tests/vvcompose_test.lua)
