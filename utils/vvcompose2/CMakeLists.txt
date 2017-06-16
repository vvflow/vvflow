cmake_minimum_required (VERSION 3.0)
set (CMAKE_CXX_STANDARD 11)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/Modules/")
project (vvcompose2)

set(__bak_LIBRARY_SUFFIXES ${CMAKE_FIND_LIBRARY_SUFFIXES})
SET(CMAKE_FIND_LIBRARY_SUFFIXES .a)
find_package(Lua REQUIRED)
find_library(LUA_LIBRARY liblua5.3.a)
SET(CMAKE_FIND_LIBRARY_SUFFIXES ${__bak_LIBRARY_SUFFIXES})
message("LIBLUA: ${LIBLUA}")
include_directories(${LUA_INCLUDE_DIR})

add_subdirectory(./../../libvvhd libvvhd)
include_directories(${LIBVVHD_INCLUDE_DIR})

#LUALIBVVD
# add_library(vvd SHARED vvcompose2.cpp)
# target_link_libraries(vvd ${LUA_LIBRARIES})

# include_directories(./)
add_executable(vvcompose2
	vvcompose2.cpp
	getset.cpp
	lua_tvec.cpp
	lua_shellscript.cpp
)
target_link_libraries(vvcompose2 ${LIBVVHD_LIBRARIES} ${LUA_LIBRARY} dl)
# target_link_libraries(vvcompose2 )

# install (TARGETS vvd DESTINATION bin)
install (TARGETS vvcompose2 DESTINATION bin)
