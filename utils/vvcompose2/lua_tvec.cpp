#include <lua.hpp>
#include <stdio.h>
#include <string.h>

#include "getset.h"
#include "lua_tvec.h"

static int tvec_abs(lua_State *L) {
    TVec* vec = lua_checkTVec(L, 1);
    lua_pushnumber(L, vec->abs());
    return 1;
}

static int tvec_abs2(lua_State *L) {
    TVec* vec = lua_checkTVec(L, 1);
    lua_pushnumber(L, vec->abs2());
    return 1;
}

static int tvec_tostring(lua_State *L) {
    TVec* vec = lua_checkTVec(L, 1);
    lua_pushfstring(L, "TVec(%f,%f)", vec->x, vec->y);
    return 1;
}

static int tvec_totable(lua_State *L) {
    TVec* vec = lua_checkTVec(L, 1);
    lua_newtable(L);
    lua_pushnumber(L, vec->x);
    lua_rawseti(L, -2, 1);
    lua_pushnumber(L, vec->y);
    lua_rawseti(L, -2, 2);

    return 1;
}

static const struct luavvd_member tvec_members[] = {
    {"x",    luavvd_getdouble, luavvd_setdouble, offsetof(TVec, x) },
    {"y",    luavvd_getdouble, luavvd_setdouble, offsetof(TVec, y) },
    {NULL, NULL, NULL, 0} /* sentinel */    
};

static const struct luavvd_method tvec_methods[] = {
    {"abs",      tvec_abs},
    {"abs2",     tvec_abs2},
    {"tostring", tvec_tostring},
    {"totable",  tvec_totable},
    {NULL, NULL}
};

static int tvec_newindex(lua_State *L) {
    TVec* vec = lua_checkTVec(L, 1);
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
    TVec* vec = lua_checkTVec(L, 1);
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

    lua_pushnil(L);
    return 1;
}

static const struct luaL_Reg luavvd_tvec [] = {
    {"__tostring", tvec_tostring},
    {"__newindex", tvec_newindex},
    {"__index", tvec_getindex},
    {NULL, NULL} /* sentinel */
};

/* PUBLIC FUNCTIONS */
/* vvvvvvvvvvvvvvvv */

int luaopen_tvec (lua_State *L) {
    luaL_newmetatable(L, "TVec"); // push 1
    luaL_setfuncs(L, luavvd_tvec, 0); /* register metamethods */
    return 0;
}

void  lua_pushTVec(lua_State *L, TVec* vec) {
    TVec** udat = (TVec**)lua_newuserdata(L, sizeof(TVec*));
    *udat = vec;
    luaL_getmetatable(L, "TVec");
    lua_setmetatable(L, -2);
}

TVec* lua_checkTVec(lua_State *L, int idx) {
    TVec** udat = (TVec**)luaL_checkudata(L, idx, "TVec");
    return *udat;
}

TVec lua_toTVecx(lua_State *L, int idx, int* isvec) {
    if (!isvec) {
        isvec = (int*)&isvec;
    }
    
    if (lua_type(L, idx) == LUA_TTABLE) {
        size_t rawlen = lua_rawlen(L, idx);
        if (rawlen != 2) 
            goto fail;

        int isnum[2];
        lua_rawgeti(L, idx, 1); // push table[1]
        lua_rawgeti(L, idx, 2); // push table[2]        
        TVec res;
        res.x = lua_tonumberx(L, -2, &isnum[0]);
        res.y = lua_tonumberx(L, -1, &isnum[1]);
        lua_pop(L, 2);
        if (!isnum[0] || !isnum[1])
            goto fail;

        *isvec = 1;
        return res;
    } else if (lua_type(L, idx) == LUA_TUSERDATA) {
        TVec** udata = (TVec**)luaL_testudata(L, idx, "TVec");
        if (!udata)
            goto fail;

        *isvec = 1;
        return **udata;
    }

fail:
    *isvec = 0;
    return TVec();
}

/* setter */
int luavvd_setTVec(lua_State *L) {
    TVec* ptr = (TVec*)lua_touserdata(L, 1);
    int isvec;
    TVec vec = lua_toTVecx(L, 2, &isvec);
    
    if (!isvec) {
        lua_pushfstring(L, "TVec needs table with two numbers");
        return 1;
    }

    *ptr = vec;
    return 0;
}

/* getter */
int luavvd_getTVec(lua_State *L) {
    TVec* vec = (TVec*)lua_touserdata(L, 1);    
    lua_pushTVec(L, vec);
    return 1;
}
