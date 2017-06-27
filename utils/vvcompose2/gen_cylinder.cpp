#include <lua.hpp>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <map>

#include "getset.h"
#include "lua_tbody.h"

extern std::map<TBody*, shared_ptr<TBody>> bodymap;

int luavvd_gen_cylinder(lua_State *L) {
    std::shared_ptr<TBody> body = std::make_shared<TBody>();
    bodymap[body.get()] = body;

    if (lua_type(L, 1)!=LUA_TTABLE) {
        luaL_error(L, "bad argument #1 for gen_cylinder (table expected, got %s)", luaL_typename(L, 1));
    }

    lua_getfield(L, 1, "N");
    lua_Integer N = luaL_checknumber(L, -1);
    TAtt att;
    att.slip = 0;

    for (size_t i=0; i<N; i++) {
        double a = double(i)/N*2*M_PI;
        att.corner.x = -0.5*sin(a);
        att.corner.y = +0.5*cos(a);
        body->alist.push_back(att);
    }

    pushTBody(L, body.get());
    return 1;
}
