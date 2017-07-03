#include <lua.hpp>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <map>

#include "getset.h"
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
    lua_Number  Ll=0.0;

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
    luaL_argcheck(L, is_ok&& isfinite(R1), 1, "'R1' must be a number");
    luaL_argcheck(L, R1 > 0, 1, "'R1' must be positive");
    lua_pushnil(L);
    lua_setfield(L, 1, "R1");

    lua_getfield(L, 1, "R2");
    R2 = lua_tonumberx(L, -1, &is_ok);
    luaL_argcheck(L, is_ok&& isfinite(R2), 1, "'R2' must be a number");
    luaL_argcheck(L, R2 > 0, 1, "'R2' must be positive");
    lua_pushnil(L);
    lua_setfield(L, 1, "R2");

    lua_getfield(L, 1, "L");
    Ll = lua_tonumberx(L, -1, &is_ok);
    luaL_argcheck(L, is_ok&& isfinite(Ll), 1, "'L' must be a number");
    luaL_argcheck(L, Ll > 0, 1, "'L' must be positive");
    lua_pushnil(L);
    lua_setfield(L, 1, "L");

    lua_getfield(L, 1, "start");
    if (!lua_isnil(L, -1)) {
        start = lua_tonumberx(L, -1, &is_ok);
        luaL_argcheck(L, is_ok, 1, "'start' must be a number");
        luaL_argcheck(L, start>=0 && start<1, 1, "'start' must be in range [0, 1)");
        lua_pushnil(L);
        lua_setfield(L, 1, "start");
    } else {
        start = 0;
    }

    lua_getfield(L, 1, "stop");
    if (!lua_isnil(L, -1)) {
        stop = lua_tonumberx(L, -1, &is_ok);
        luaL_argcheck(L, is_ok, 1, "'stop' must be a number");
        luaL_argcheck(L, stop>0 && stop<=1, 1, "'stop' must be in range (0, 1]");
        lua_pushnil(L);
        lua_setfield(L, 1, "stop");
    } else {
        stop = 1;
    }

    lua_getfield(L, 1, "gap");
    if (!lua_isnil(L, -1)) {
        gap = lua_tonumberx(L, -1, &is_ok);
        luaL_argcheck(L, is_ok&& isfinite(gap), 1, "'gap' must be a number");
        luaL_argcheck(L, gap > 0, 1, "'gap' must be positive");
        lua_pushnil(L);
        lua_setfield(L, 1, "gap");
    } else {
        gap = 0;
    }

    lua_pushnil(L);
    if (lua_next(L, 1)) {
        const char* param = lua_tostring(L, -2);
        lua_pushfstring(L, "excess parameter '%s'", param);
        luaL_argerror(L, 1, lua_tostring(L, -1));
    }

    // # some calculations
    double alpha = asin((R2-R1)/Ll);
    double phi = M_PI/2+alpha;
    double perimeter = 2 * ( (M_PI-phi)*R1 + (phi)*R2 + Ll*cos(alpha) );
    if (!dl) {
        dl = perimeter / N;
    } else {
        N = int( perimeter/dl + 0.5);
    }
    int N_start = 2*int( (M_PI-phi)*R1/dl +0.5);
    int N_stop =  2*int(      (phi)*R2/dl +0.5);
    int N_side =  (N - N_start - N_stop)/2;

    double r1 = R1 + start*(R2-R1);
    double r2 = R1 + stop*(R2-R1);
    double c1 = start*Ll;
    double c2 = stop*Ll;
    gap = max(dl, gap);

    double phi1, phi2;
    if (stop == 1) {
        phi1 = 0;
        phi2 = phi;
    } else {
        phi1 = M_PI;
        phi2 = phi+acos(r2/(r2+gap));
        r2 = r2+gap;
    }

    gen_arc (body->alist, TVec(c2, 0), r2, -phi1, -phi2,              N_stop/2, stop<1);
    gen_line(body->alist, TVec(c2, 0)+r2*TVec(cos(phi2), -sin(phi2)),
                          TVec(c1, 0)+r1*TVec(cos(phi),  -sin(phi)),  N_side,   false);
    gen_arc (body->alist, TVec(c1, 0), r1, 2*M_PI-phi, phi,           N_start,  start>0);
    gen_line(body->alist, TVec(c1, 0)+r1*TVec(cos(phi),  +sin(phi)),
                          TVec(c2, 0)+r2*TVec(cos(phi2), +sin(phi2)), N_side,   false);
    gen_arc (body->alist, TVec(c2, 0), r2, +phi2, +phi1,              N_stop/2, stop<1);

    body->doUpdateSegments();
    body->doFillProperties();

    bodymap[body.get()] = body;
    pushTBody(L, body.get());
    return 1;
}
