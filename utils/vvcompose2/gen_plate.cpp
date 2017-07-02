#include <lua.hpp>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <map>

#include "getset.h"
#include "gen_body.h"
#include "lua_tbody.h"

extern std::map<TBody*, shared_ptr<TBody>> bodymap;

int luavvd_gen_plate(lua_State *L) {
    std::shared_ptr<TBody> body = std::make_shared<TBody>();

    luaL_checktype(L, 1, LUA_TTABLE);

    int is_ok;
    lua_Integer N=0;
    lua_Number  dl=0.0;
    lua_Number  R1=0.0;
    lua_Number  R2=0.0;
    lua_Number  start=0.0;
    lua_Number  stop=0.0;
    lua_Number  gap=0.0;
    lua_Number  L=0.0;

    lua_getfield(L, 1, "N");
    if (!lua_isnil(L, -1)) {
        N = lua_tointegerx(L, -1, &is_ok);
        luaL_argcheck(L, is_ok, 1, "'N' must be a number");
        luaL_argcheck(L, N > 0, 1, "'N' must be positive");
    }
    lua_pushnil(L);
    lua_setfield(L, 1, "N");

    lua_getfield(L, 1, "dl");
    if (!lua_isnil(L, -1)) {
        dl = lua_tonumberx(L, -1, &is_ok);
        luaL_argcheck(L, is_ok && isfinite(dl), 1, "'dl' must be a number");
        luaL_argcheck(L, dl>0, 1, "'dl' must be positive");
        luaL_argcheck(L, N==0, 1, "'N' and 'dl' are mutually exclusive");
    } else {
        luaL_argcheck(L, N!=0, 1, "either 'N' or dl' must be specified");
    }
    lua_pushnil(L);
    lua_setfield(L, 1, "dl");

    lua_getfield(L, 1, "R1");
    R1 = lua_tonumberx(L, -1, &is_ok);
    luaL_argcheck(L, is_ok&& isfinite(r), 1, "'R1' must be a number");
    luaL_argcheck(L, R > 0, 1, "'R1' must be positive");
    lua_pushnil(L);
    lua_setfield(L, 1, "R1");

    lua_getfield(L, 1, "R2");
    R2 = lua_tonumberx(L, -1, &is_ok);
    luaL_argcheck(L, is_ok&& isfinite(r), 1, "'R2' must be a number");
    luaL_argcheck(L, R > 0, 1, "'R2' must be positive");
    lua_pushnil(L);
    lua_setfield(L, 1, "R2");

    lua_getfield(L, 1, "L");
    L = lua_tonumberx(L, -1, &is_ok);
    luaL_argcheck(L, is_ok&& isfinite(r), 1, "'L' must be a number");
    luaL_argcheck(L, R > 0, 1, "'L' must be positive");
    lua_pushnil(L);
    lua_setfield(L, 1, "L");

    lua_pushnil(L);
    if (lua_next(L, 1)) {
        const char* param = lua_tostring(L, -2);
        lua_pushfstring(L, "excess parameter '%s'", param);
        luaL_argerror(L, 1, lua_tostring(L, -1));
    }

    TAtt att;
    att.slip = 0;
    if (!N) {
        N = ceil(2*M_PI*r/dl);
    }

    for (size_t i=0; i<N; i++) {
        double a = double(i)/N*2*M_PI;
        att.corner.x = -r*sin(a);
        att.corner.y = +r*cos(a);
        body->alist.push_back(att);
    }

    bodymap[body.get()] = body;
    pushTBody(L, body.get());
    return 1;
}
