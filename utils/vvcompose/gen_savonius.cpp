#include <lua.hpp>

#include "lua_tbody.h"
#include "gen_body.h"

int luavvd_gen_savonius(lua_State *L) {
    luaL_checktype(L, 1, LUA_TTABLE);

    lua_Integer N=0;
    lua_Number  dl=0.0;
    lua_Number  R=0.0;
    lua_Number  h=0.0;

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

    R = get_param(L, "R", "positive", std::numeric_limits<double>::min());
    h = get_param(L, "h", "positive", std::numeric_limits<double>::min());

    luaL_argcheck(L, (R>h), 1, "'R' must be larger than 'h'");

    lua_pushnil(L);
    if (lua_next(L, 1)) {
        const char* param = lua_tostring(L, -2);
        lua_pushfstring(L, "excess parameter '%s'", param);
        luaL_argerror(L, 1, lua_tostring(L, -1));
    }

    std::shared_ptr<TBody> body = std::make_shared<TBody>();

    lua_Number R2 = R/2;
    lua_Number h2 = h/2;

    if (N>0) {
        //  Convert N to dl
        lua_Number circumference = M_PI*(4*R+h);
        dl = circumference/N;
    }

    gen_arc_dl(body->alist, TVec(R   , 0), R-h2, M_PI  , 2*M_PI, dl);   // inner shell, right
    gen_arc_dl(body->alist, TVec(R*2 , 0), h2  , M_PI  , 0     , dl);   // shell end, right
    gen_arc_dl(body->alist, TVec(R   , 0), R+h2, 2*M_PI, M_PI  , dl);   // outer shell, right
    gen_arc_dl(body->alist, TVec(-R  , 0), R-h2, 0     , M_PI  , dl);   // inner shell, left
    gen_arc_dl(body->alist, TVec(-R*2, 0), h2  , 2*M_PI, M_PI  , dl);   // shell end, left
    gen_arc_dl(body->alist, TVec(-R  , 0), R+h2, M_PI  , 0     , dl);   // outer shell, left

    body->doUpdateSegments();
    body->doFillProperties();
    pushTBody(L, body);
    return 1;
}