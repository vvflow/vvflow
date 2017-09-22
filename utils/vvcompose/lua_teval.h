#pragma once

#include "TEval.hpp"
#include <lua.hpp>

int luaopen_teval (lua_State *L);

/* push userdata with (TEval*) and set metatable "LibVVD.TEval" */
void  pushTEavl(lua_State *L, TEval* script);
/* get userdata (TEval*) and check metatable is "LibVVD.TEval" */
TEval* checkTEval(lua_State *L, int idx);

