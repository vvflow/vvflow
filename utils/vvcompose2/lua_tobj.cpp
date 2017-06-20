#include <lua.hpp>
#include <stdio.h>
#include <string.h>

#include "getset.h"
#include "lua_tobj.h"

static int tobj_tostring(lua_State *L) {
    TObj* obj = lua_checkTObj(L, 1);
    lua_pushfstring(L, "TObj(%f,%f,%f)", obj->r.x, obj->r.y, obj->g);
    return 1;
}

static const struct luavvd_member tobj_members[] = {
    {"r",    luavvd_getTVec,   luavvd_setTVec,   offsetof(TObj, r) },
    {"g",    luavvd_getdouble, luavvd_setdouble, offsetof(TObj, g) },
    {"v",    luavvd_getTVec,   luavvd_setTVec,   offsetof(TObj, v) },
    {NULL, NULL, NULL, 0} /* sentinel */    
};

static const struct luavvd_method tobj_methods[] = {
    {"2string", tobj_tostring},
    {NULL, NULL}
};

static int tobj_newindex(lua_State *L) {
    TObj* obj = lua_checkTObj(L, 1);
    const char *name = luaL_checkstring(L, 2);

    for (auto f = tobj_members; f->name; f++) {
        if (strcmp(name, f->name)) continue;
        lua_pushcfunction(L, f->setter);
        lua_pushlightuserdata(L, (char*)obj + f->offset);
        lua_pushvalue(L, 3);
        lua_call(L, 2, 1);
        if (!lua_isnil(L, -1)) {
            luaL_error(L, "bad value for TObj.%s (%s)", name, lua_tostring(L, -1));
        }
        return 0;
    }

    luaL_error(L, "TObj can not assign '%s'", name);
    return 0;
}

static int tobj_getindex(lua_State *L) {
    TObj* obj = lua_checkTObj(L, 1);
    const char *name = luaL_checkstring(L, 2);

    for (auto f = tobj_members; f->name; f++) {
        if (strcmp(name, f->name)) continue;
        lua_pushcfunction(L, f->getter);
        lua_pushlightuserdata(L, (char*)obj + f->offset);
        lua_call(L, 1, 1);
        return 1;
    }

    for (auto f = tobj_methods; f->name; f++) {
        if (strcmp(name, f->name)) continue;
        lua_pushcfunction(L, f->func);
        return 1;
    }

    return luaL_error(L, "TObj has no member '%s'", name);
}

static const struct luaL_Reg luavvd_tobj [] = {
    {"__tostring", tobj_tostring},
    {"__newindex", tobj_newindex},
    {"__index",    tobj_getindex},
    {NULL, NULL} /* sentinel */
};

/* PUBLIC FUNCTIONS */
/* vvvvvvvvvvvvvvvv */

int luaopen_tobj (lua_State *L) {
    luaL_newmetatable(L, "TObj"); // push 1
    luaL_setfuncs(L, luavvd_tobj, 0); /* register metamethods */
    return 0;
}

void lua_pushTObj(lua_State *L, TObj* obj) {
    TObj** udat = (TObj**)lua_newuserdata(L, sizeof(void*));
    *udat = obj;
    luaL_getmetatable(L, "TObj");
    lua_setmetatable(L, -2);
}

TObj* lua_checkTObj(lua_State *L, int idx) {
    TObj** udat = (TObj**)luaL_checkudata(L, idx, "TObj");
    return *udat;
}


TObj lua_toTObjx(lua_State *L, int idx, int* isobj) {
    if (!isobj) {
        isobj = (int*)&isobj;
    }
    
    if (lua_type(L, idx) == LUA_TTABLE) {
        size_t rawlen = lua_rawlen(L, idx);
        if (rawlen != 3) 
            goto fail;

        int isnum[3];
        lua_rawgeti(L, idx, 1); // push table[1]
        lua_rawgeti(L, idx, 2); // push table[2]
        lua_rawgeti(L, idx, 3); // push table[3]
        TObj res;
        res.r.x = lua_tonumberx(L, -3, &isnum[0]);
        res.r.y = lua_tonumberx(L, -2, &isnum[1]);
        res.g =   lua_tonumberx(L, -1, &isnum[2]);
        lua_pop(L, 3);
        for (int i=0; i<3; i++) {
            if (!isnum[i])
                goto fail;
        }

        *isobj = 1;
        return res;
    } else if (lua_type(L, idx) == LUA_TUSERDATA) {
        TObj** udata = (TObj**)luaL_testudata(L, idx, "TObj");
        if (!udata)
            goto fail;

        *isobj = 1;
        return **udata;
    }

fail:
    *isobj = 0;
    return TObj();
}

/* setter */
int luavvd_setTObj(lua_State *L) {
    TObj* ptr = (TObj*)lua_touserdata(L, 1);
    int isobj;
    TObj obj = lua_toTObjx(L, 2, &isobj);

    if (!isobj) {
        lua_pushfstring(L, "TObj needs table with three numbers");
        return 1;
    }

    *ptr = obj;
    return 0;
}

/* getter */
int luavvd_getTObj(lua_State *L) {
    TObj* obj = (TObj*)lua_touserdata(L, 1);    
    lua_pushTObj(L, obj);
}
