#include "getset.h"
#include "lua_tbody.h"

#include <lua.hpp>
#include <cmath>
#include <cstdio>
#include <cstring>

using std::isnan;
using std::isfinite;

lua_Number get_param(
    lua_State *L,
    const char* param,
    const char* range_err,
    lua_Number min,
    lua_Number max,
    lua_Number dflt
) {
    int is_ok = 0;
    lua_Number result;

    lua_getfield(L, 1, param);
    if (!lua_isnil(L, -1)) {
        result = lua_tonumberx(L, -1, &is_ok);
        lua_pop(L, 1);
    } else if (!isnan(dflt)) {
        lua_pop(L, 1);
        return dflt;
    }

    lua_pushnil(L);
    lua_setfield(L, 1, param);
    
    if (!is_ok) {
        lua_pushfstring(L, "'%s' must be a number", param);
    } else if (!isfinite(result)) {
        lua_pushfstring(L, "'%s' must be finite", param);
    } else if (result < min || result > max) {
        lua_pushfstring(L, "'%s' must be %s", param, range_err);
    } else {
        return result;
    }

    return luaL_argerror(L, 1, lua_tostring(L, -1));
}

void gen_seg_N(
    std::vector<TAtt>& alist,
    TVec p1,
    TVec p2,
    size_t N,
    uint32_t slip
) {

    for (size_t i=0; i<N; i++) {
        alist.emplace_back(p1 + (p2-p1)*i/N, slip);
    }
}

void gen_seg_dl(
    std::vector<TAtt>& alist,
    TVec p1,
    TVec p2,
    double dl,
    uint32_t slip
) {
    gen_seg_N(
        alist,
        p1,
        p2,
        floor((p2-p1).abs()/dl + 0.5),
        slip
    );
}

void gen_arc_N(
    std::vector<TAtt>& alist,
    TVec c,
    double R,
    double a1,
    double a2,
    size_t N,
    uint32_t slip
) {
    for (size_t i=0; i<N; i++) {
        double ai = a1 + (a2-a1)*double(i)/double(N);
        TVec p = c + TVec(R*cos(ai), R*sin(ai));
        alist.emplace_back(p, slip);
    }
}

void gen_arc_dl(
    std::vector<TAtt>& alist,
    TVec c,
    double R,
    double a1,
    double a2,
    double dl,
    uint32_t slip
) {
    gen_arc_N(
        alist,
        c,
        R,
        a1,
        a2,
        floor(R*fabs(a2-a1)/dl+0.5),
        slip
    );
}
