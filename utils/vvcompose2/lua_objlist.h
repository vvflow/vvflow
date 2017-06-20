#include "core.h"

int luaopen_objlist (lua_State *L);

/* push userdata with (vector*) and set metatable "ObjList" */
void  pushObjList(lua_State *L, vector<TObj>* li);
/* get userdata (vector*) and check metatable is "ObjList" */
vector<TObj>* checkObjList(lua_State *L, int idx);

