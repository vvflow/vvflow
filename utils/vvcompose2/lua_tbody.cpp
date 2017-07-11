#include <lua.hpp>
#include <stdio.h>
#include <string.h>
#include <map>

#include "getset.h"
#include "lua_tbody.h"
#include "lua_tvec.h"
#include "lua_tvec3d.h"
#include "lua_shellscript.h"

std::map<TBody*, shared_ptr<TBody>> bodymap;

static int tbody_move_r(lua_State *L) {
    TBody* body = checkTBody(L, 1);

    // TVec vec = TVec();
    TVec3D move_vec = TVec3D();
    lua_pushcfunction(L, luavvd_setTVec);
    lua_pushlightuserdata(L, (char*)&move_vec.r);
    lua_pushvalue(L, 2);
    lua_call(L, 2, 1);
    if (!lua_isnil(L, -1)) {
        luaL_error(L, "bad argument #1 for TBody.move_r (%s)", lua_tostring(L, -1));
    }

    body->move(move_vec, move_vec);

    return 0;
}

static int tbody_move_o(lua_State *L) {
    TBody* body = checkTBody(L, 1);
    lua_Number val = luaL_checknumber(L, 2);

    TVec3D move_vec = TVec3D(0, 0, val);
    body->move(move_vec, move_vec);

    return 0;
}

static int tbody_move_d(lua_State *L) {
    TBody* body = checkTBody(L, 1);
    lua_Number val = luaL_checknumber(L, 2);

    TVec3D move_vec = TVec3D(0, 0, val*C_PI/180.0);
    body->move(move_vec, move_vec);

    return 0;
}

static int tbody_get_surface(lua_State *L) {
    lua_pushnumber(L, checkTBody(L, 1)->get_surface());
    return 1;
}
static int tbody_get_area(lua_State *L) {
    lua_pushnumber(L, checkTBody(L, 1)->get_area());
    return 1;
}
static int tbody_get_moi_c(lua_State *L) {
    lua_pushnumber(L, checkTBody(L, 1)->get_moi_c());
    return 1;
}
static int tbody_get_com(lua_State *L) {
    TBody* body = checkTBody(L, 1);
    TVec com = body->get_com();
    lua_newtable(L);
    lua_pushnumber(L, com.x);
    lua_rawseti(L, -2, 1);
    lua_pushnumber(L, com.y);
    lua_rawseti(L, -2, 2);
    return 1;
}
static int tbody_get_axis(lua_State *L) {
    TBody* body = checkTBody(L, 1);
    TVec axis = body->get_axis();
    lua_newtable(L);
    lua_pushnumber(L, axis.x);
    lua_rawseti(L, -2, 1);
    lua_pushnumber(L, axis.y);
    lua_rawseti(L, -2, 2);
    return 1;
}

static int tbody_totable(lua_State *L) {
    TBody* body = checkTBody(L, 1);
    lua_newtable(L);
    for (int i=0; i<body->alist.size(); i++) {
        lua_pushTVec(L, &body->alist[i].corner);
        lua_rawseti(L, -2, i+1);
    }
    return 1;
}

static const struct luavvd_member tbody_members[] = {
    {"label",           luavvd_getstring,      luavvd_setstring,      0 },
    {"holder_pos",      luavvd_getTVec3D,      luavvd_setTVec3D,      0 },
    {"delta_pos",       luavvd_getTVec3D,      luavvd_setTVec3D,      0 },
    {"speed",           luavvd_getTVec3D,      luavvd_setTVec3D,      0 },
    {"collision_min",   luavvd_getTVec3D,      luavvd_setTVec3D,      0 },
    {"collision_max",   luavvd_getTVec3D,      luavvd_setTVec3D,      0 },
    {"spring_const",    luavvd_getTVec3D,      luavvd_setTVec3D,      0 },
    {"spring_damping",  luavvd_getTVec3D,      luavvd_setTVec3D,      0 },
    {"holder_vx",       luavvd_getShellScript, luavvd_setShellScript, 0 },
    {"holder_vy",       luavvd_getShellScript, luavvd_setShellScript, 0 },
    {"holder_vo",       luavvd_getShellScript, luavvd_setShellScript, 0 },
    {"density",         luavvd_getdouble,      luavvd_setdouble,      0 },
    {"bounce",          luavvd_getdouble,      luavvd_setdouble,      0 },
    {"special_segment", luavvd_getint32,       luavvd_setint32,       0 },
    // {"general_slip",    luavvd_getslip,        luavvd_setslip,        0 },
    // {"root_body",       luavvd_getrootbody,    luavvd_setrootbody,    0 },
    {NULL, NULL, NULL, 0} /* sentinel */    
};

#define BIND_MEMBER(NAME, MEMBER) \
    else if (!strcmp(name, NAME)) { \
        lua_pushlightuserdata(L, (char*)&body->MEMBER); \
    }

#define PUSH_MEMBER() \
    if (0) ; \
    BIND_MEMBER("label",           label) \
    BIND_MEMBER("holder_pos",      holder) \
    BIND_MEMBER("delta_pos",       dpos) \
    BIND_MEMBER("speed",           speed_slae) \
    BIND_MEMBER("collision_min",   collision_min) \
    BIND_MEMBER("collision_max",   collision_max) \
    BIND_MEMBER("spring_const",    kspring) \
    BIND_MEMBER("spring_damping",  damping) \
    BIND_MEMBER("holder_vx",       speed_x) \
    BIND_MEMBER("holder_vy",       speed_y) \
    BIND_MEMBER("holder_vo",       speed_o) \
    BIND_MEMBER("density",         density) \
    BIND_MEMBER("bounce",          bounce) \
    BIND_MEMBER("special_segment", special_segment_no)

static const struct luavvd_method tbody_methods[] = {
    {"move_r", tbody_move_r},
    {"move_o", tbody_move_o},
    {"move_d", tbody_move_d},
    {"get_arclen", tbody_get_surface},
    {"get_area", tbody_get_area},
    {"get_moi_c", tbody_get_moi_c},
    {"get_com", tbody_get_com},
    {"get_axis", tbody_get_axis},
    {"totable", tbody_totable},
    {NULL, NULL}
};

static int tbody_newindex(lua_State *L) {
    TBody* body = checkTBody(L, 1);
    const char *name = luaL_checkstring(L, 2);

    for (auto f = tbody_members; f->name; f++) {
        if (strcmp(name, f->name)) continue;
        lua_pushcfunction(L, f->setter);
        PUSH_MEMBER();
        lua_pushvalue(L, 3);
        lua_call(L, 2, 1);
        if (!lua_isnil(L, -1)) {
            luaL_error(L, "bad value for TBody.%s (%s)", name, lua_tostring(L, -1));
        }
        return 0;
    }

    return luaL_error(L, "TBody can not assign '%s'", name);
}

static int tbody_getindex(lua_State *L) {
    TBody* body = checkTBody(L, 1);
    const char *name = luaL_checkstring(L, 2);

    for (auto f = tbody_members; f->name; f++) {
        if (strcmp(name, f->name)) continue;
        lua_pushcfunction(L, f->getter);
        PUSH_MEMBER();
        lua_call(L, 1, 1);
        return 1;
    }

    for (auto f = tbody_methods; f->name; f++) {
        if (strcmp(name, f->name)) continue;
        lua_pushcfunction(L, f->func);
        return 1;
    }

    lua_pushnil(L);
    return 1;
}

#undef BIND_MEMBER

static int tbody_getlen(lua_State *L) {
    TBody* body = checkTBody(L, 1);
    lua_pushinteger(L, body->size());
    return 1;
}

static const struct luaL_Reg luavvd_tbody [] = {
    {"__newindex", tbody_newindex},
    {"__index",    tbody_getindex},
    {"__len",      tbody_getlen},
    {NULL, NULL} /* sentinel */
};

/* PUBLIC FUNCTIONS */
/* vvvvvvvvvvvvvvvv */

int luaopen_tbody (lua_State *L) {
    luaL_newmetatable(L, "TBody"); // push 1
    luaL_setfuncs(L, luavvd_tbody, 0); /* register metamethods */
    return 0;
}

void  pushTBody(lua_State *L, TBody* body) {
    TBody** udat = (TBody**)lua_newuserdata(L, sizeof(TBody*));
    *udat = body;
    luaL_getmetatable(L, "TBody");
    lua_setmetatable(L, -2);
}

TBody* checkTBody(lua_State *L, int idx) {
    TBody** udat = (TBody**)luaL_checkudata(L, idx, "TBody");
    return *udat;
}

int luavvd_load_body(lua_State *L) {
    std::shared_ptr<TBody> body = std::make_shared<TBody>();
    bodymap[body.get()] = body;

    const char* fname = luaL_checkstring(L, 1);
    int err = body->load_txt(fname);
    if (err) {
        luaL_error(L, "can not load '%s' (%s)\n", fname, strerror(err));
    }

    pushTBody(L, body.get());
    return 1;
}

// int luavvd_setTBody(lua_State *L) {
//     TBody* vec = (TBody*)lua_touserdata(L, 1);
    
//     int isnum[2];
//     switch (lua_type(L, 2)) {
//     case LUA_TTABLE:
//         for (size_t rawlen = lua_rawlen(L, 2); rawlen != 2; ) {
//             lua_pushfstring(L, "TBody needs table with two elements, got %d", rawlen);
//             return 1;
//         }

//         lua_geti(L, 2, 1); // push table[1]
//         lua_geti(L, 2, 2); // push table[2]        
//         vec->x = lua_tonumberx(L, -2, &isnum[0]);
//         vec->y = lua_tonumberx(L, -1, &isnum[1]);
//         if (!isnum[0] || !isnum[1]) {
//             const char *typ1 = luaL_typename(L, -2);
//             const char *typ2 = luaL_typename(L, -1);
//             lua_pushfstring(L, "TBody needs two numbers, got %s and %s", typ1, typ2);
//             return 1;
//         }
//         return 0;
//     case LUA_TUSERDATA:
//         TBody** udata = (TBody**)luaL_testudata(L, 2, "TVec");

//         if (!udata) {
//             const char *typ;
//             if (luaL_getmetafield(L, 2, "__name") == LUA_TSTRING)
//                 typ = lua_tostring(L, -1);
//             else
//                 typ = "userdata";
//             lua_pushfstring(L, "table or TVec expected, got %s", typ);
//             return 1;    
//         }

//         *vec = **udata;
//         return 0;
//     }
    
//     const char *typ = luaL_typename(L, 2);
//     lua_pushfstring(L, "table or TVec expected, got %s", typ);
//     return 1;
// }

// int luavvd_getTBody(lua_State *L) {
//     TVec* vec = (TVec*)lua_touserdata(L, 1);    
//     pushTVec(L, vec);
// }

void gen_line(std::vector<TAtt>& alist, TVec p1, TVec p2, size_t N, uint32_t slip) {
    TAtt att;
    att.slip = slip;
    for (size_t i=0; i<N; i++) {
        att.corner = p1 + (p2-p1)*i/N;
        alist.push_back(att);
    }
}

void gen_arc(std::vector<TAtt>& alist, TVec c, double R, double a1, double a2, size_t N, uint32_t slip) {
    TAtt att;
    att.slip = slip;
    for (size_t i=0; i<N; i++) {
        double ai = a1 + (a2-a1)*double(i)/double(N);
        att.corner = c + TVec(R*cos(ai), R*sin(ai));
        alist.push_back(att);
    }
}
