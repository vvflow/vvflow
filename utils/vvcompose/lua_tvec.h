#include "core.h"

int luaopen_tvec (lua_State *L);

/* push userdata with (TVec*) and set metatable "TVec" */
void  lua_pushTVec(lua_State *L, TVec* vec);
/* get userdata (TVec*) and check metatable is "TVec" */
TVec* lua_checkTVec(lua_State *L, int idx);
/* get value at index and convert it to TVec */
TVec  lua_toTVecx(lua_State *L, int idx, int* isvec);
