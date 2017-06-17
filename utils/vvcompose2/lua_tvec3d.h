#include "core.h"

int luaopen_tvec3d (lua_State *L);

/* push userdata with (TVec3D*) and set metatable "LibVVD.TVec3D" */
void  pushTVec3D(lua_State *L, TVec3D* vec);
/* get userdata (TVec3D*) and check metatable is "LibVVD.TVec3D" */
TVec3D* checkTVec3D(lua_State *L, int idx);

int luavvd_setTVec3D(lua_State *L);
int luavvd_getTVec3D(lua_State *L);
