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
        luaL_error(L, "bad argument #2 for TBody.move_r (%s)", lua_tostring(L, -1));
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

static const struct luavvd_member tbody_members[] = {
    {"label",           luavvd_getstring,      luavvd_setstring,      offsetof(TBody, label) },
    {"holder_pos",      luavvd_getTVec3D,      luavvd_setTVec3D,      offsetof(TBody, holder) },
    {"delta_pos",       luavvd_getTVec3D,      luavvd_setTVec3D,      offsetof(TBody, dpos) },
    {"speed",           luavvd_getTVec3D,      luavvd_setTVec3D,      offsetof(TBody, speed_slae) },
    {"collision_min",   luavvd_getTVec3D,      luavvd_setTVec3D,      offsetof(TBody, collision_min) },
    {"collision_max",   luavvd_getTVec3D,      luavvd_setTVec3D,      offsetof(TBody, collision_max) },
    {"spring_const",    luavvd_getTVec3D,      luavvd_setTVec3D,      offsetof(TBody, kspring) },
    {"spring_damping",  luavvd_getTVec3D,      luavvd_setTVec3D,      offsetof(TBody, damping) },
    {"holder_vx",       luavvd_getShellScript, luavvd_setShellScript, offsetof(TBody, speed_x) },
    {"holder_vy",       luavvd_getShellScript, luavvd_setShellScript, offsetof(TBody, speed_y) },
    {"holder_vo",       luavvd_getShellScript, luavvd_setShellScript, offsetof(TBody, speed_o) },
    {"density",         luavvd_getdouble,      luavvd_setdouble,      offsetof(TBody, density) },
    {"bounce",          luavvd_getdouble,      luavvd_setdouble,      offsetof(TBody, bounce) },
    {"special_segment", luavvd_getint32,       luavvd_setint32,       offsetof(TBody, special_segment_no) },
    // {"general_slip",    luavvd_getslip,        luavvd_setslip,        0 },
    // {"root_body",       luavvd_getrootbody,    luavvd_setrootbody,    0 },
    {NULL, NULL, NULL, 0} /* sentinel */    
};

static const struct luavvd_method tbody_methods[] = {
    {"move_r", tbody_move_r},
    {"move_o", tbody_move_o},
    {"move_d", tbody_move_d},
    {NULL, NULL}
};

static int tbody_newindex(lua_State *L) {
    TBody* body = checkTBody(L, 1);
    const char *name = luaL_checkstring(L, 2);

    for (auto f = tbody_members; f->name; f++) {
        if (strcmp(name, f->name)) continue;
        lua_pushcfunction(L, f->setter);
        lua_pushlightuserdata(L, (char*)body + f->offset);
        lua_pushvalue(L, 3);
        lua_call(L, 2, 1);
        if (!lua_isnil(L, -1)) {
            luaL_error(L, "bad value for TBody.%s (%s)", name, lua_tostring(L, -1));
        }
        return 0;
    }

    luaL_error(L, "TBody can not assign '%s'", name);
    return 0;
}

static int tbody_getindex(lua_State *L) {
    TBody* body = checkTBody(L, 1);
    const char *name = luaL_checkstring(L, 2);

    for (auto f = tbody_members; f->name; f++) {
        if (strcmp(name, f->name)) continue;
        lua_pushcfunction(L, f->getter);
        lua_pushlightuserdata(L, (char*)body + f->offset);
        lua_call(L, 1, 1);
        return 1;
    }

    for (auto f = tbody_methods; f->name; f++) {
        if (strcmp(name, f->name)) continue;
        lua_pushcfunction(L, f->func);
        return 1;
    }

    return luaL_error(L, "TBody has no member '%s'", name);
}

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
