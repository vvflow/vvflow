#include <lua.hpp>
#include <stdio.h>
#include <string.h>

#include "getset.h"
#include "tvec.h"

static int tvec_abs2(lua_State *L) {
    TVec* vec = checkTVec(L, 1);
    lua_pushnumber(L, vec->abs2());
    return 1;
}

static int tvec_tostring(lua_State *L) {
    TVec* vec = checkTVec(L, 1);
    lua_pushfstring(L, "TVec(%f,%f)", vec->x, vec->y);
    return 1;
}

static const struct luavvd_member tvec_members[] = {
    {"x",    luavvd_getdouble, luavvd_setdouble, offsetof(TVec, x) },
    {"y",    luavvd_getdouble, luavvd_setdouble, offsetof(TVec, y) },
    {NULL, NULL, NULL, 0} /* sentinel */    
};

static const struct luavvd_method tvec_methods[] = {
    {"abs2",    tvec_abs2},
    {"2string", tvec_tostring},
    {NULL, NULL}
};

static int tvec_newindex(lua_State *L) {
    TVec* vec = checkTVec(L, 1);
    const char *name = luaL_checkstring(L, 2);

    for (auto f = tvec_members; f->name; f++) {
        if (strcmp(name, f->name)) continue;
        lua_pushcfunction(L, f->setter);
        lua_pushlightuserdata(L, (char*)vec + f->offset);
        lua_pushvalue(L, 3);
        lua_call(L, 2, 1);
        if (!lua_isnil(L, -1)) {
            luaL_error(L, "bad value for TVec.%s (%s)", name, lua_tostring(L, -1));
        }
        return 0;
    }

    luaL_error(L, "TVec can not assign '%s'", name);
    return 0;
}

static int tvec_getindex(lua_State *L) {
    TVec* vec = checkTVec(L, 1);
    const char *name = luaL_checkstring(L, 2);

    for (auto f = tvec_members; f->name; f++) {
        if (strcmp(name, f->name)) continue;
        lua_pushcfunction(L, f->getter);
        lua_pushlightuserdata(L, (char*)vec + f->offset);
        lua_call(L, 1, 1);
        return 1;
    }

    for (auto f = tvec_methods; f->name; f++) {
        if (strcmp(name, f->name)) continue;
        lua_pushcfunction(L, f->func);
        return 1;
    }

    return luaL_error(L, "TVec has no member '%s'", name);
}

static const struct luaL_Reg luavvd_tvec [] = {
    // {"abs2", tvec_abs2},
    // {"tvec2string", TVec2string},
    {"__tostring", tvec_tostring},
    {"__newindex", tvec_newindex},
    {"__index", tvec_getindex},
    // {"__metaname", tvec_getindex},
    {NULL, NULL} /* sentinel */
};

/* PUBLIC FUNCTIONS */
/* vvvvvvvvvvvvvvvv */

int luaopen_tvec (lua_State *L) {
    luaL_newmetatable(L, "TVec"); // push 1
    luaL_setfuncs(L, luavvd_tvec, 0); /* register metamethods */
    return 0;
}

void  pushTVec(lua_State *L, TVec* vec) {
    TVec** udat = (TVec**)lua_newuserdata(L, sizeof(TVec*));
    *udat = vec;
    luaL_getmetatable(L, "TVec");
    lua_setmetatable(L, -2);
}

TVec* checkTVec(lua_State *L, int idx) {
    TVec** udat = (TVec**)luaL_checkudata(L, idx, "TVec");
    return *udat;
}

int luavvd_setTVec(lua_State *L) {
    TVec* vec = (TVec*)lua_touserdata(L, 1);
    
    int isnum[2];
    switch (lua_type(L, 2)) {
    case LUA_TTABLE:
        for (size_t rawlen = lua_rawlen(L, 2); rawlen != 2; ) {
            lua_pushfstring(L, "TVec needs table with two elements, got %d", rawlen);
            return 1;
        }

        lua_geti(L, 2, 1); // push table[1]
        lua_geti(L, 2, 2); // push table[2]        
        vec->x = lua_tonumberx(L, -2, &isnum[0]);
        vec->y = lua_tonumberx(L, -1, &isnum[1]);
        if (!isnum[0] || !isnum[1]) {
            const char *typ1 = luaL_typename(L, -2);
            const char *typ2 = luaL_typename(L, -1);
            lua_pushfstring(L, "TVec needs two numbers, got %s and %s", typ1, typ2);
            return 1;
        }
        return 0;
    case LUA_TUSERDATA:
        TVec** udata = (TVec**)luaL_testudata(L, 2, "TVec");

        if (!udata) {
            const char *typ;
            if (luaL_getmetafield(L, 2, "__name") == LUA_TSTRING)
                typ = lua_tostring(L, -1);
            else
                typ = "userdata";
            lua_pushfstring(L, "table or TVec expected, got %s", typ);
            return 1;    
        }

        *vec = **udata;
        return 0;
    }
    
    const char *typ = luaL_typename(L, 2);
    lua_pushfstring(L, "table or TVec expected, got %s", typ);
    return 1;
}

int luavvd_getTVec(lua_State *L) {
    TVec* vec = (TVec*)lua_touserdata(L, 1);    
    pushTVec(L, vec);
}
