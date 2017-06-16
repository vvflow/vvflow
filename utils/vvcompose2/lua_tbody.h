#include "core.h"

int luaopen_tbody (lua_State *L);

/* push userdata with (TBody*) and set metatable "LibVVD.TBody" */
void  pushTBody(lua_State *L, TBody* body);
/* get userdata (TBody*) and check metatable is "LibVVD.TBody" */
TBody* checkTBody(lua_State *L, int idx);

// int luavvd_setTBody(lua_State *L);
// int luavvd_getTBody(lua_State *L);
