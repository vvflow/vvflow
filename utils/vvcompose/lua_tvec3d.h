#pragma once

#include "TVec3D.hpp"
#include <lua.hpp>

int luaopen_tvec3d (lua_State *L);

/* push userdata with (TVec3D*) and set metatable "TVec3D" */
void    lua_pushTVec3D(lua_State *L, TVec3D* vec);
/* get userdata (TVec3D*) and check metatable is "TVec3D" */
TVec3D* lua_checkTVec3D(lua_State *L, int idx);
/* get value at index and convert it to TVec3D */
TVec3D  lua_toTVec3Dx(lua_State *L, int idx, int* isvec);
