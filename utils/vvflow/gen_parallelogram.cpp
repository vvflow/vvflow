#include <lua.hpp>

#include "lua_tbody.h"
#include "gen_body.h"

int luavvd_gen_parallelogram(lua_State *L) {
    luaL_checktype(L, 1, LUA_TTABLE);

    lua_Integer N=0;
    lua_Number  dl=0.0;
    lua_Number  d_=0.0;
    lua_Number  L_=0.0;
    lua_Number  H_=0.0;

    N = get_param(L, "N", "positive",
        /*min*/ 1,
        /*max*/ std::numeric_limits<lua_Integer>::max(),
        /*default*/ 0);
    dl = get_param(L, "dl", "positive",
        /*min*/ std::numeric_limits<lua_Number>::min(),
        /*max*/ std::numeric_limits<lua_Number>::infinity(),
        /*default*/ 0);
    luaL_argcheck(L, (N>0)|(dl>0), 1, "either 'N' or dl' must be specified");
    luaL_argcheck(L, (N>0)^(dl>0), 1, "'N' and 'dl' are mutually exclusive");

    L_ = get_param(L, "L",  "positive", std::numeric_limits<lua_Number>::min());
    H_ = get_param(L, "H",  "positive", std::numeric_limits<lua_Number>::min());
    d_ = get_param(L, "d", "in range (0, 180)",
        /*min*/ std::numeric_limits<lua_Number>::min(),
        /*max*/ 180*(1 - std::numeric_limits<lua_Number>::epsilon()),
        /*default*/ 0);

    lua_pushnil(L);
    if (lua_next(L, 1)) {
        const char* param = lua_tostring(L, -2);
        lua_pushfstring(L, "excess parameter '%s'", param);
        luaL_argerror(L, 1, lua_tostring(L, -1));
    }

    // # some calculations
    double alpha = d_ / 180. * M_PI;
    double L2 = H_ / tan(alpha);
    if (!dl) {
        double slen = 2 * ( L_ + H_/sin(alpha));
        dl = slen / N;
    }

    std::shared_ptr<TBody> body = std::make_shared<TBody>();

    gen_seg_dl(body->alist, TVec(L_   ,  0), TVec(    0,  0), dl);
    gen_seg_dl(body->alist, TVec(    0,  0), TVec(   L2, H_), dl);
    gen_seg_dl(body->alist, TVec(   L2, H_), TVec(L_+L2, H_), dl);
    gen_seg_dl(body->alist, TVec(L_+L2, H_), TVec(L_   ,  0), dl);

    body->doUpdateSegments();
    body->doFillProperties();

    pushTBody(L, body);
    return 1;
}
