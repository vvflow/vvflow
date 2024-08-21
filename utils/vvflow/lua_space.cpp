#include "lua_space.h"
#include "TSpace.hpp"
#include "XStreamfunction.hpp"
#include "getset.h"

#include <cstring>

static int space_load(lua_State *L) {
    Space **ptr = (Space**)luaL_checkudata(L, 1, "S");
    Space *S = *ptr;
    const char* fname = luaL_checkstring(L, 2);
    S->load(fname);
    return 0;
}

static int space_save(lua_State *L) {
    Space **ptr = (Space**)luaL_checkudata(L, 1, "S");
    Space *S = *ptr;
    const char* fname = luaL_checkstring(L, 2);
    try {
        S->save(fname);
    } catch (const std::invalid_argument &e) {
        return luaL_error(L, e.what());
    }
    return 0;
}

static int space_xstreamfunction(lua_State *L) {
    Space **ptr = (Space**)luaL_checkudata(L, 1, "S");
    Space *S = *ptr;

    TVec vec = {};
    lua_pushcfunction(L, luavvd_setTVec);
    lua_pushlightuserdata(L, (char*)&vec);
    lua_pushvalue(L, 2);
    lua_call(L, 2, 1);
    if (!lua_isnil(L, -1)) {
        luaL_error(L, "bad argument #1 for TSpace.XStreamfunction (%s)", lua_tostring(L, -1));
    }

    double ret = XStreamfunction::streamfunction(*S, vec);
    lua_pushnumber(L, ret);
    return 1;
}

static const struct luavvd_member space_members[] = {
    {"caption", luavvd_getstring,      luavvd_setstring,      offsetof(Space, caption) },
    {"re",      luavvd_getdouble,      luavvd_setdouble,      offsetof(Space, re) },
    {"finish",  luavvd_getdouble,      luavvd_setdouble,      offsetof(Space, finish) },
    {"inf_g",   luavvd_getdouble,      luavvd_setdouble,      offsetof(Space, inf_g) },
    {"inf_vx",  luavvd_getTEval,       luavvd_setTEval,       offsetof(Space, inf_vx) },
    {"inf_vy",  luavvd_getTEval,       luavvd_setTEval,       offsetof(Space, inf_vy) },
    {"gravity", luavvd_getTVec,        luavvd_setTVec,        offsetof(Space, gravity) },

    {"time",       luavvd_getTTime, luavvd_setTTime, offsetof(Space, time) },
    {"dt",         luavvd_getTTime, luavvd_setTTime, offsetof(Space, dt) },
    {"dt_save",    luavvd_getTTime, luavvd_setTTime, offsetof(Space, dt_save) },
    {"dt_streak",  luavvd_getTTime, luavvd_setTTime, offsetof(Space, dt_streak) },
    {"dt_profile", luavvd_getTTime, luavvd_setTTime, offsetof(Space, dt_profile) },

    {"body_list",         luavvd_getBodyList, NULL, offsetof(Space, BodyList) },
    {"vort_list",          luavvd_getObjList, NULL, offsetof(Space, VortexList) },
    {"sink_list",          luavvd_getObjList, NULL, offsetof(Space, SourceList) },
    {"streak_source_list", luavvd_getObjList, NULL, offsetof(Space, StreakSourceList) },
    {"streak_domain_list", luavvd_getObjList, NULL, offsetof(Space, StreakList) },

    {NULL, NULL, NULL, 0} /* sentinel */    
};

static const struct luavvd_method space_methods[] = {
    {"load", space_load},
    {"save", space_save},
    {"XStreamfunction", space_xstreamfunction},
    {NULL, NULL}
} ;

static int space_newindex(lua_State *L) {
    Space **ptr = (Space**)luaL_checkudata(L, 1, "S");
    Space *S = *ptr;
    const char *name = luaL_checkstring(L, 2);

    for (auto f = space_members; f->name; f++) {
        if (strcmp(name, f->name)) continue;
        if (!f->setter) break;
        lua_pushcfunction(L, f->setter);
        lua_pushlightuserdata(L, (char*)S + f->offset);
        lua_pushvalue(L, 3);
        lua_call(L, 2, 1);
        if (!lua_isnil(L, -1)) {
            luaL_error(L, "bad value for S.%s (%s)", name, lua_tostring(L, -1));
        }
        return 0;
    }

    luaL_error(L, "S can not assign '%s'", name);
    return 0;
}

static int space_getindex(lua_State *L) {
    Space **ptr = (Space**)luaL_checkudata(L, 1, "S");
    Space *S = *ptr;
    const char *name = luaL_checkstring(L, 2);

    for (auto f = space_members; f->name; f++) {
        if (strcmp(name, f->name)) continue;
        lua_pushcfunction(L, f->getter);
        lua_pushlightuserdata(L, (char*)S + f->offset);
        lua_call(L, 1, 1);
        return 1;
    }

    for (auto f = space_methods; f->name; f++) {
        if (strcmp(name, f->name)) continue;
        lua_pushcfunction(L, f->func);
        return 1;
    }

    lua_pushnil(L);
    return 1;
}

static const struct luaL_Reg luavvd_space [] = {
    {"__newindex", space_newindex},
    {"__index",    space_getindex},
    {NULL, NULL} /* sentinel */
};

/* PUBLIC FUNCTIONS */
/* vvvvvvvvvvvvvvvv */

int luaopen_space (lua_State *L) {
    static Space S;
    Space** ptr = (Space**)lua_newuserdata(L, sizeof(Space*)); // push 1 (Space)
    *ptr = &S;
    luaL_newmetatable(L, "S"); // push 2 (Space.mt)
    luaL_setfuncs(L, luavvd_space, 0);
    lua_setmetatable(L, -2); // pop 2
    lua_setglobal(L, "S"); // pop 1
    return 0;
}
