#include <lua.hpp>

#include "lua_tbody.h"
#include "gen_body.h"
#include "elementary.h"

int luavvd_gen_ellipse(lua_State *L) {
    luaL_checktype(L, 1, LUA_TTABLE);

    lua_Integer N=0;
    lua_Number  dl=0.0;
    lua_Number  Rx=0.0;
    lua_Number  Ry=0.0;

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

    Rx = get_param(L, "Rx", "positive",      std::numeric_limits<double>::min());
    Ry = get_param(L, "Ry", "positive <= Rx", std::numeric_limits<double>::min(), Rx);

    lua_pushnil(L);
    if (lua_next(L, 1)) {
        const char* param = lua_tostring(L, -2);
        lua_pushfstring(L, "excess parameter '%s'", param);
        luaL_argerror(L, 1, lua_tostring(L, -1));
    }

    if (!dl) {
        // Ramanujan (II)
        double h = sqr((Rx-Ry)/(Rx+Ry));
        double slen = M_PI*(Rx+Ry)*(1. + 3*h/(10+sqrt(4-3*h)));
        dl = slen/N;
    }
    luaL_argcheck(L, dl<Ry/1.9, 1, "discretization too low");

    std::vector<double> semiell;
    for (double a=asin(dl/(2*Ry)); a<M_PI-asin(dl/(2*Ry)); ) {
        semiell.push_back(a);
        double da1 = dl / sqrt( sqr(Rx*sin(a)) + sqr(Ry*cos(a)) );
        double da2 = dl / sqrt( sqr(Rx*sin(a+da1)) + sqr(Ry*cos(a+da1)) );
        a += (da1+da2)/2;
    }

    std::shared_ptr<TBody> body = std::make_shared<TBody>();
    N = semiell.size()-1;
    // emplace bottom half
    for (double i=0; i<=N; i++) {
        double a = semiell[i]*(1.-i/N) + (M_PI-semiell[N-i])*(i/N);
        body->alist.emplace_back(Rx*cos(a), -Ry*sin(a));
    }
    // emplace top half
    for (double i=0; i<=N; i++) {
        double a = semiell[i]*(1.-i/N) + (M_PI-semiell[N-i])*(i/N);
        body->alist.emplace_back(-Rx*cos(a), Ry*sin(a));
    }

    body->doUpdateSegments();
    body->doFillProperties();
    pushTBody(L, body);
    return 1;
}
