#include "core.h"

int luaopen_tvec (lua_State *L);

/* push userdata with (TVec*) and set metatable "LibVVD.TVec" */
void  pushTVec(lua_State *L, TVec* vec);
/* get userdata (TVec*) and check metatable is "LibVVD.TVec" */
TVec* checkTVec(lua_State *L, int idx);

int luavvd_setTVec(lua_State *L);
int luavvd_getTVec(lua_State *L);
