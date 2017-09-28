#include <lua.hpp>

#include "lua_tbody.h"
#include "gen_body.h"

using std::numeric_limits;
static const auto dblmin = numeric_limits<lua_Number>::min();
static const auto dbleps = numeric_limits<lua_Number>::epsilon();

int luavvd_gen_chamber_box(lua_State *L) {
    luaL_checktype(L, 1, LUA_TTABLE);

    lua_Number  dln=0.0;
    lua_Number  dls=0.0;
    lua_Number  h=0.0;
    lua_Number  d=0.0;
    lua_Number  _L=0.0;
    lua_Number  _H=0.0;
    lua_Number  _D=0.0;

    dln = get_param(L, "dln", "positive", dblmin);
    dls = get_param(L, "dls", "positive", dblmin);
    _L =  get_param(L, "L",   "positive", dblmin);
    _H =  get_param(L, "H",   "positive", dblmin);
    _D =  get_param(L, "D",   "positive < L", dblmin, _L*(1-dbleps));
    h  =  get_param(L, "h",   "positive", dblmin);
    d  =  get_param(L, "d",   "positive < L-D", dblmin, (_L-_D)*(1-dbleps));

    std::shared_ptr<TBody> body = std::make_shared<TBody>();
    lua_Number D2 = _D/2+d/2;
    lua_Number L2 = _L/2;

    gen_seg_dl(body->alist, TVec(0.   ,   _H), TVec(-L2  ,   _H), dls, true);  // inner top left
    gen_seg_dl(body->alist, TVec(-L2  ,   _H), TVec(-L2  ,   0.), dls, true);  // inner left
    gen_seg_dl(body->alist, TVec(-L2  ,   0.), TVec(-D2  ,   0.), dln, false);  // inner bottom left
    gen_arc_dl(body->alist, TVec(-D2  , -d/2), d/2, +M_PI*1/2., 0, dln, false); // left hole edge
    gen_arc_dl(body->alist, TVec(-D2  , -d/2), d/2, 0, -M_PI*1/2., dln, false); // left hole edge
    gen_seg_dl(body->alist, TVec(-D2  , -d  ), TVec(-L2-h,   -d), dls, true);  // outer bottom left
    gen_seg_dl(body->alist, TVec(-L2-h, -d  ), TVec(-L2-h, _H+h), dls, true);  // outer left
    gen_seg_dl(body->alist, TVec(-L2-h, _H+h), TVec(+L2+h, _H+h), dls, true);  // outer top
    gen_seg_dl(body->alist, TVec(+L2+h, _H+h), TVec(+L2+h,   -d), dls, true);  // outer right
    gen_seg_dl(body->alist, TVec(+L2+h, -d  ), TVec(+D2  ,   -d), dls, true);  // outer bottom right
    gen_arc_dl(body->alist, TVec(+D2  , -d/2), d/2, M_PI*3/2., M_PI, dln, false); // right hole edge
    gen_arc_dl(body->alist, TVec(+D2  , -d/2), d/2, M_PI, M_PI*1/2., dln, false); // right hole edge
    gen_seg_dl(body->alist, TVec(+D2  ,   0.), TVec(+L2  ,   0.), dln, false); // inner bottom right
    gen_seg_dl(body->alist, TVec(+L2  ,   0.), TVec(+L2  ,   _H), dls, true);  // inner right
    gen_seg_dl(body->alist, TVec(+L2  ,   _H), TVec(0.   ,   _H), dls, true);  // inner top right

    body->doUpdateSegments();
    body->doFillProperties();

    pushTBody(L, body);
    return 1;
}
