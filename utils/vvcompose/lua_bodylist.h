#include "core.h"

int luaopen_bodylist (lua_State *L);

/* push userdata with (vector*) and set metatable "BodyList" */
void  pushBodyList(lua_State *L, vector<shared_ptr<TBody>>* li);
/* get userdata (vector*) and check metatable is "BodyList" */
vector<shared_ptr<TBody>>* checkBodyList(lua_State *L, int idx);

