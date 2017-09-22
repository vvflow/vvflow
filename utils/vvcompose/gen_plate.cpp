#include <lua.hpp>

#include "lua_tbody.h"
#include "gen_body.h"

int luavvd_gen_plate(lua_State *L) {
    luaL_checktype(L, 1, LUA_TTABLE);

    lua_Integer N=0;
    lua_Number  dl=0.0;
    lua_Number  R1=0.0;
    lua_Number  R2=0.0;
    lua_Number  start=0.0;
    lua_Number  stop=0.0;
    lua_Number  gap=0.0;
    lua_Number  Ll=0.0;

    N = get_param(L, "N", "positive",
        /*min*/ 1,
        /*max*/ std::numeric_limits<lua_Integer>::max(),
        /*default*/ 0);
    dl = get_param(L, "dl", "positive",
        /*min*/ std::numeric_limits<double>::min(),
        /*max*/ std::numeric_limits<double>::infinity(),
        /*default*/ 0);
    luaL_argcheck(L, (N>0)|(dl>0), 1, "either 'N' or dl' must be specified");
    luaL_argcheck(L, (N>0)^(dl>0), 1, "'N' and 'dl' are mutually exclusive");

    R1 = get_param(L, "R1", ">= 0", 0);
    R2 = get_param(L, "R2", ">= 0", 0);
    luaL_argcheck(L, (R1>0)|(R2>0), 1, "either 'R1' or 'R2' must be > 0");
    Ll = get_param(L, "L",  "positive", std::numeric_limits<double>::min());
    start = get_param(L, "start", "in range [0, 1)",
        /*min*/ 0,
        /*max*/ 1 - std::numeric_limits<double>::epsilon(),
        /*default*/ 0);
    stop  = get_param(L, "stop",  "in range (0, 1]",
        /*min*/ std::numeric_limits<double>::min(),
        /*max*/ 1,
        /*default*/ 1);
    gap   = get_param(L, "gap",   "positive",
        /*min*/ std::numeric_limits<double>::min(),
        /*max*/ std::numeric_limits<double>::infinity(),
        /*default*/ 0
    );

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
    gap = std::max(dl, gap);

    double phi1, phi2;
    if (stop == 1) {
        phi1 = 0;
        phi2 = phi;
    } else {
        phi1 = M_PI;
        phi2 = phi+acos(r2/(r2+gap));
        r2 = r2+gap;
    }

    std::shared_ptr<TBody> body = std::make_shared<TBody>();

    gen_arc_N(body->alist, TVec(c2, 0), r2, -phi1, -phi2,              N_stop/2, stop<1);
    gen_seg_N(body->alist, TVec(c2, 0)+r2*TVec(cos(phi2), -sin(phi2)),
                           TVec(c1, 0)+r1*TVec(cos(phi),  -sin(phi)),  N_side,   false);
    gen_arc_N(body->alist, TVec(c1, 0), r1, 2*M_PI-phi, phi,           N_start,  start>0);
    gen_seg_N(body->alist, TVec(c1, 0)+r1*TVec(cos(phi),  +sin(phi)),
                           TVec(c2, 0)+r2*TVec(cos(phi2), +sin(phi2)), N_side,   false);
    gen_arc_N(body->alist, TVec(c2, 0), r2, +phi2, +phi1,              N_stop/2, stop<1);

    body->doUpdateSegments();
    body->doFillProperties();

    pushTBody(L, body);
    return 1;
}
