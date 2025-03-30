cmake_minimum_required (VERSION 3.0)
project (vvflow)

include_directories(${LIBVVHD_INCLUDE_DIRS})

add_executable(vvflow
    vvflow.cpp
    optparse.cpp
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
    communicate.cpp
)
target_link_libraries(vvflow vvhd dl)

add_custom_command(
    TARGET vvflow POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E create_symlink vvflow vvcompose
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    VERBATIM)

if(WITH_MANPAGES)
set(MAN_MD ${CMAKE_CURRENT_SOURCE_DIR}/vvflow.1.md)
set(MAN_ROFF ${CMAKE_CURRENT_BINARY_DIR}/vvflow.1)
set(MAN_HTML ${CMAKE_CURRENT_BINARY_DIR}/vvflow.1.html)
add_custom_command(
    OUTPUT ${MAN_ROFF} ${MAN_HTML}
    COMMAND ${RONN} --roff --pipe ${MAN_MD} > ${MAN_ROFF}
    COMMAND ${RONN} --html --pipe ${MAN_MD} > ${MAN_HTML}
    DEPENDS ${MAN_MD}
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    COMMENT "Building manpage ${MAN_ROFF}"
    VERBATIM)
add_custom_target(vvflow.man ALL DEPENDS ${MAN_ROFF} ${MAN_HTML})
endif()

install (TARGETS vvflow DESTINATION bin)
install (FILES ${CMAKE_CURRENT_BINARY_DIR}/vvcompose DESTINATION bin)
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

add_test(vvcompose_test vvflow ${CMAKE_BINARY_DIR}/tests/vvcompose_test.lua)
