#include <lua.hpp>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <map>

#include "getset.h"
#include "lua_tbody.h"

int luavvd_gen_cylinder(lua_State *L) {
    luaL_checktype(L, 1, LUA_TTABLE);

    int is_ok;
    lua_Integer N=0;
    lua_Number  dl=0.0;
    lua_Number  R=0.0;

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

    lua_getfield(L, 1, "R");
    R = lua_tonumberx(L, -1, &is_ok);
    luaL_argcheck(L, is_ok&& isfinite(R), 1, "'R' must be a number");
    luaL_argcheck(L, R > 0, 1, "'R' must be positive");
    lua_pushnil(L);
    lua_setfield(L, 1, "R");

    lua_pushnil(L);
    if (lua_next(L, 1)) {
        const char* param = lua_tostring(L, -2);
        lua_pushfstring(L, "excess parameter '%s'", param);
        luaL_argerror(L, 1, lua_tostring(L, -1));
    }

    if (!N) {
        N = ceil(2*M_PI*R/dl);
    }

    std::shared_ptr<TBody> body = std::make_shared<TBody>();
    gen_arc(body->alist, TVec(0, 0), R, 2*M_PI, 0, N);

    body->doUpdateSegments();
    body->doFillProperties();
    pushTBody(L, body);
    return 1;
}
