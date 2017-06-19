#include <lua.hpp>
#include <stdio.h>
#include <string.h>

#include "getset.h"

static const struct luavvd_method objlist_methods[] = {
    // {"2string", tvec3d_tostring},
    // vlist:append()
    // vlist:clear()
    // vlist:load()
    // blist["cyl"] = gen_cyl
    // blist["plate"] = gen_plate
    // blist["body00"] = gis
    // blist["plate"] = nil
    // blist:append(gen_cyl)
    // blist:remove(cyl)

    {NULL, NULL}
};

static int objlist_newindex(lua_State *L) {
    TVec3D* vec = checkTVec3D(L, 1);
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

static int objlist_getindex(lua_State *L) {
    vector<TObj>* li = checkObjList(L, 1);
    lua_Integer = luaL_checkinteger(L, 2);

    if (lua_isinteger(L, 2)) {

    } else if (lua_isstring(L, 2)) {
        const char *name = luaL_checkstring(L, 2);
        for (auto f = tvec3d_methods; f->name; f++) {
            if (strcmp(name, f->name)) continue;
            lua_pushcfunction(L, f->func);
            return 1;
        }
        return luaL_error(L, "TList has no member '%s'", name);
    } else {
        return luaL_error(L, "TList invalid key (string or integer expected, got %s", luaL_typename(L, 2));
    }
    switch (lua_type(L, 2)) {
    case LUA_TNUMBER:

    }
    for (auto f = tvec3d_members; f->name; f++) {
        if (strcmp(name, f->name)) continue;
        lua_pushcfunction(L, f->getter);
        lua_pushlightuserdata(L, (char*)vec + f->offset);
        lua_call(L, 1, 1);
        return 1;
    }


}

static const struct luaL_Reg luavvd_objlist [] = {
    {"__newindex", objlist_newindex},
    {"__index", objlist_getindex},
    {"__len", objlist_getlen},
    {NULL, NULL} /* sentinel */
};

/* PUBLIC FUNCTIONS */
/* vvvvvvvvvvvvvvvv */

int luaopen_objlist (lua_State *L) {
    luaL_newmetatable(L, "ObjList"); // push 1
    luaL_setfuncs(L, luavvd_objlist, 0); /* register metamethods */
    return 0;
}

void  pushObjList(lua_State *L, vector<TObj>* li) {
    vector<TObj>** udat = (vector<TObj>**)lua_newuserdata(L, sizeof(void*));
    *udat = li;
    luaL_getmetatable(L, "ObjList");
    lua_setmetatable(L, -2);
}

vector<TObj>* checkObjList(lua_State *L, int idx) {
    vector<TObj>** udat = (vector<TObj>**)luaL_checkudata(L, idx, "ObjList");
    return *udat;
}

int luavvd_setObjList(lua_State *L) {
    lua_pushfstring(L, "TList is static, it can not be assigned");
    return 1;
}

int luavvd_getObjList(lua_State *L) {
    vector<TObj>* li = (vector<TObj>*)lua_touserdata(L, 1);    
    pushObjList(L, li);
}
