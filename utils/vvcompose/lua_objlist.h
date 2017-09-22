#pragma once

#include "TObj.hpp"
#include <vector>

int luaopen_objlist (lua_State *L);

/* push userdata with (vector*) and set metatable "ObjList" */
void  pushObjList(lua_State *L, std::vector<TObj>* li);
/* get userdata (vector*) and check metatable is "ObjList" */
std::vector<TObj>* checkObjList(lua_State *L, int idx);

