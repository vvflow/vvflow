#include <lua.hpp>

#include "lua_tbody.h"
#include "gen_body.h"

int luavvd_gen_chamber_gpj(lua_State *L) {
    luaL_checktype(L, 1, LUA_TTABLE);

    lua_Number  dln=0.0;
    lua_Number  dls=0.0;
    lua_Number  R0=0.0;
    lua_Number  X0=0.0;
    lua_Number  X1=0.0;
    lua_Number  X2=0.0;
    lua_Number  X3, Y3;
    lua_Number  X4, Y4;
    lua_Number  d=0.0;
    lua_Number  _L=0.0;
    lua_Number  _H=0.0;

    dln = get_param(L, "dln", "positive", std::numeric_limits<lua_Number>::min());
    dls = get_param(L, "dls", "positive", std::numeric_limits<lua_Number>::min());
    R0 =  get_param(L, "R0",  "positive", std::numeric_limits<lua_Number>::min());
    d  =  get_param(L, "d",   "in range [0, 90)", 0, 90*(1-std::numeric_limits<lua_Number>::epsilon()));
    X0 =  get_param(L, "X0");
    X1 =  get_param(L, "X1",  ">= X0", X0);
    X2 =  get_param(L, "X2",  ">= X1", X1);
    X3 =  get_param(L, "X3",  ">= X2", X2);
    Y3 =  R0+(X3-X2)*tan(d*M_PI/180.0);
    _L =  get_param(L, "L",   "greater than chamber length", (X3-X0+R0)*(1+std::numeric_limits<lua_Number>::epsilon()) );
    _H =  get_param(L, "H",   "greater than chamber width",  2*Y3*(1+std::numeric_limits<lua_Number>::epsilon()) );
    X4 = X3 - _L;
    Y4 = _H/2;

    lua_pushnil(L);
    if (lua_next(L, 1)) {
        const char* param = lua_tostring(L, -2);
        lua_pushfstring(L, "excess parameter '%s'", param);
        luaL_argerror(L, 1, lua_tostring(L, -1));
    }

    // # some calculations
    lua_Number dls2 = std::min(dls, (Y4-Y3)/5);

    std::shared_ptr<TBody> body = std::make_shared<TBody>();

    gen_seg_dl(body->alist, TVec(X4, -Y4), TVec(X4, +Y4), dls,  true);  // bbox left
    gen_seg_dl(body->alist, TVec(X4, +Y4), TVec(X3, +Y4), dls,  true);  // bbox top
    gen_seg_dl(body->alist, TVec(X3, +Y4), TVec(X3, +Y3), dls2, true);  // bbox right top
    gen_seg_dl(body->alist, TVec(X3, +Y3), TVec(X2, +R0), dln,  false); // expanding channel
    gen_seg_dl(body->alist, TVec(X2, +R0), TVec(X1, +R0), dln,  false); // straight channel no-slip
    gen_seg_dl(body->alist, TVec(X1, +R0), TVec(X0, +R0), dls,  true);  // straight channel slip
    gen_arc_dl(body->alist, TVec(X0, 0.0), R0, M_PI*1/2., M_PI*3/2., dls, true); // circular prechamber
    gen_seg_dl(body->alist, TVec(X0, -R0), TVec(X1, -R0), dls,  true);  // straight channel slip
    gen_seg_dl(body->alist, TVec(X1, -R0), TVec(X2, -R0), dln,  false); // straight channel no-slip
    gen_seg_dl(body->alist, TVec(X2, -R0), TVec(X3, -Y3), dln,  false); // expanding channel
    gen_seg_dl(body->alist, TVec(X3, -Y3), TVec(X3, -Y4), dls2, true);  // bbox right bottom
    gen_seg_dl(body->alist, TVec(X3, -Y4), TVec(X4, -Y4), dls,  true);  // bbox bottom

    body->doUpdateSegments();
    body->doFillProperties();

    pushTBody(L, body);
    return 1;
}
