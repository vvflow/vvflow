#pragma once

#include "TObj.hpp"
#include <lua.hpp>

int luaopen_tobj(lua_State *L);

/* push userdata with (TObj*) and set metatable "TObj" */
void  lua_pushTObj(lua_State *L, TObj* obj);
/* get userdata (TObj*) and check metatable is "TObj" */
TObj* lua_checkTObj(lua_State *L, int idx);
/* get value at index and convert it to TObj */
TObj  lua_toTObjx(lua_State *L, int idx, int* isobj);

