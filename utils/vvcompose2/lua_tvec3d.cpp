#include <lua.hpp>
#include <stdio.h>
#include <string.h>

#include "getset.h"
#include "lua_tvec.h"
#include "lua_tvec3d.h"

static int tvec3d_tostring(lua_State *L) {
    TVec3D* vec = lua_checkTVec3D(L, 1);
    lua_pushfstring(L, "TVec3D(%f,%f,%f)", vec->r.x, vec->r.y, vec->o);
    return 1;
}

static int tvec3d_totable(lua_State *L) {
    TVec3D* vec = lua_checkTVec3D(L, 1);
    lua_newtable(L);
    lua_pushnumber(L, vec->r.x);
    lua_rawseti(L, -2, 1);
    lua_pushnumber(L, vec->r.y);
    lua_rawseti(L, -2, 2);
    lua_pushnumber(L, vec->o);
    lua_rawseti(L, -2, 3);

    return 1;
}

int luavvd_setdegree(lua_State *L) {
    double* dbl = (double*)lua_touserdata(L, 1);
    
    int isnum;
    lua_Number val = lua_tonumberx(L, 2, &isnum);
    if (!isnum) {
        lua_pushfstring(L, "number expected, got %s", luaL_typename(L, 2));
        return 1;
    }

    *dbl = val*C_PI/180.0;
    return 0;
}

int luavvd_getdegree(lua_State *L) {
    double* dbl = (double*)lua_touserdata(L, 1);
    lua_pushnumber(L, *dbl*180.0/C_PI);
    return 1;
}

static const struct luavvd_member tvec3d_members[] = {
    {"r",    luavvd_getTVec,   luavvd_setTVec,   offsetof(TVec3D, r) },
    {"o",    luavvd_getdouble, luavvd_setdouble, offsetof(TVec3D, o) },
    {"d",    luavvd_getdegree, luavvd_setdegree, offsetof(TVec3D, o) },
    {NULL, NULL, NULL, 0} /* sentinel */    
};

static const struct luavvd_method tvec3d_methods[] = {
    {"tostring", tvec3d_tostring},
    {"totable",  tvec3d_totable},
    {NULL, NULL}
};

static int tvec3d_newindex(lua_State *L) {
    TVec3D* vec = lua_checkTVec3D(L, 1);
    const char *name = luaL_checkstring(L, 2);

    for (auto f = tvec3d_members; f->name; f++) {
        if (strcmp(name, f->name)) continue;
        lua_pushcfunction(L, f->setter);
        lua_pushlightuserdata(L, (char*)vec + f->offset);
        lua_pushvalue(L, 3);
        lua_call(L, 2, 1);
        if (!lua_isnil(L, -1)) {
            luaL_error(L, "bad value for TVec3D.%s (%s)", name, lua_tostring(L, -1));
        }
        return 0;
    }

    luaL_error(L, "TVec3D can not assign '%s'", name);
    return 0;
}

static int tvec3d_getindex(lua_State *L) {
    TVec3D* vec = lua_checkTVec3D(L, 1);
    const char *name = luaL_checkstring(L, 2);

    for (auto f = tvec3d_members; f->name; f++) {
        if (strcmp(name, f->name)) continue;
        lua_pushcfunction(L, f->getter);
        lua_pushlightuserdata(L, (char*)vec + f->offset);
        lua_call(L, 1, 1);
        return 1;
    }

    for (auto f = tvec3d_methods; f->name; f++) {
        if (strcmp(name, f->name)) continue;
        lua_pushcfunction(L, f->func);
        return 1;
    }

    lua_pushnil(L);
    return 1;
}

static const struct luaL_Reg luavvd_tvec3d [] = {
    {"__tostring", tvec3d_tostring},
    {"__newindex", tvec3d_newindex},
    {"__index", tvec3d_getindex},
    {NULL, NULL} /* sentinel */
};

/* PUBLIC FUNCTIONS */
/* vvvvvvvvvvvvvvvv */

int luaopen_tvec3d (lua_State *L) {
    luaL_newmetatable(L, "TVec3D"); // push 1
    luaL_setfuncs(L, luavvd_tvec3d, 0); /* register metamethods */
    return 0;
}

void  lua_pushTVec3D(lua_State *L, TVec3D* vec) {
    TVec3D** udat = (TVec3D**)lua_newuserdata(L, sizeof(TVec3D*));
    *udat = vec;
    luaL_getmetatable(L, "TVec3D");
    lua_setmetatable(L, -2);
}

TVec3D* lua_checkTVec3D(lua_State *L, int idx) {
    TVec3D** udat = (TVec3D**)luaL_checkudata(L, idx, "TVec3D");
    return *udat;
}

TVec3D lua_toTVec3Dx(lua_State *L, int idx, int* isvec) {
    if (!isvec) {
        isvec = (int*)&isvec;
    }
    
    if (lua_type(L, idx) == LUA_TTABLE) {
        size_t rawlen = lua_rawlen(L, idx);
        if (rawlen != 3)
            goto fail;

        int isnum[3];
        lua_rawgeti(L, idx, 1); // push table[1]
        lua_rawgeti(L, idx, 2); // push table[2]
        lua_rawgeti(L, idx, 3); // push table[3]
        TVec3D res;
        res.r.x = lua_tonumberx(L, -3, &isnum[0]);
        res.r.y = lua_tonumberx(L, -2, &isnum[1]);
        res.o   = lua_tonumberx(L, -1, &isnum[2]);
        lua_pop(L, 3);
        if (!isnum[0] || !isnum[1] || !isnum[2])
            goto fail;

        *isvec = 1;
        return res;
    } else if (lua_type(L, idx) == LUA_TUSERDATA) {
        TVec3D** udata = (TVec3D**)luaL_testudata(L, idx, "TVec3D");
        if (!udata)
            goto fail;

        *isvec = 1;
        return **udata;
    }

fail:
    *isvec = 0;
    return TVec3D();
}

/* setter */
int luavvd_setTVec3D(lua_State *L) {
    TVec3D* ptr = (TVec3D*)lua_touserdata(L, 1);
    int isvec;
    TVec3D vec = lua_toTVec3Dx(L, 2, &isvec);
    
    if (!isvec) {
        lua_pushfstring(L, "TVec3D needs table with three numbers");
        return 1;
    }

    *ptr = vec;
    return 0;
}

/* getter */
int luavvd_getTVec3D(lua_State *L) {
    TVec3D* vec = (TVec3D*)lua_touserdata(L, 1);    
    lua_pushTVec3D(L, vec);
}
