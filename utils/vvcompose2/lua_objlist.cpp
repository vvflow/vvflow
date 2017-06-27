#include <lua.hpp>
#include <stdio.h>
#include <string.h>
#include <cstring>

#include "getset.h"
#include "lua_tobj.h"
#include "lua_objlist.h"

static int objlist_append(lua_State *L) {
    vector<TObj> *li = checkObjList(L, 1);
    int isobj;

    for (int i=2; i<=lua_gettop(L); i++) {
        TObj obj = lua_toTObjx(L, i, &isobj);
        if (!isobj) {
            luaL_argerror(L, i, "expected TObj");
        }
        li->push_back(obj);
    }

    return 0;
}

static int objlist_clear(lua_State *L) {
    vector<TObj> *li = checkObjList(L, 1);
    li->clear();
    return 0;
}

static int objlist_load_txt(lua_State *L) {
    vector<TObj> *li = checkObjList(L, 1);
    const char* fname = luaL_checkstring(L, 2);
    int err = Space::load_list_txt(*li, fname);
    if (err) {
        luaL_error(L, "can not load '%s' (%s)", fname, strerror(err));
    }
    return 0;
}

static int objlist_load_bin(lua_State *L) {
    vector<TObj> *li = checkObjList(L, 1);
    const char* fname = luaL_checkstring(L, 2);
    Space::load_list_bin(*li, fname);
    return 0;
}

static int objlist_getlen(lua_State *L) {
    vector<TObj> *li = checkObjList(L, 1);
    lua_pushinteger(L, li->size());
    return 1;
}

static int objlist_totable(lua_State *L) {
    vector<TObj> *li = checkObjList(L, 1);
    lua_newtable(L);
    for (int i=0; i<li->size(); i++) {
        lua_pushTObj(L, &li->at(i));
        lua_rawseti(L, -2, i+1);
    }
    return 1;
}

static const struct luavvd_method objlist_methods[] = {
    {"append",   objlist_append},
    {"clear",    objlist_clear},
    {"load",     objlist_load_txt},
    {"load_bin", objlist_load_bin},
    {"totable",  objlist_totable},
    // blist["cyl"] = gen_cyl
    // blist["plate"] = gen_plate
    // blist["body00"] = gis
    // blist["plate"] = nil
    // blist:append(gen_cyl)
    // blist:remove(cyl)

    {NULL, NULL}
};

static int objlist_newindex(lua_State *L) {
    luaL_error(L, "TList can not assign anything");
    return 0;
}

// int stackDump(lua_State *L);
static int objlist_getindex(lua_State *L) {
    vector<TObj>* li = checkObjList(L, 1);

    if (lua_isnumber(L, 2)) {
        lua_Integer idx = lua_tointeger(L, 2)-1;
        if (idx >= 0 && idx < li->size()) {
            lua_pushTObj(L, &li->at(idx));
            return 1;
        }
    } else if (lua_isstring(L, 2)) {
        const char *name = luaL_checkstring(L, 2);
        for (auto f = objlist_methods; f->name; f++) {
            if (strcmp(name, f->name)) continue;
            lua_pushcfunction(L, f->func);
            return 1;
        }
    }

    lua_pushnil(L);
    return 1;
}

static int objlist_getnext(lua_State *L) {
    vector<TObj>* li = checkObjList(L, 1);

    int idx = luaL_optinteger(L, 2, 0);
    if (idx >= li->size()) {
        lua_pushnil(L);
        return 1;
    } else {
        lua_pushinteger(L, idx+1);
        lua_pushTObj(L, &li->at(idx));
        return 2;
    }
}

static int objlist_getpairs(lua_State *L) {
    lua_pushcfunction(L, objlist_getnext);
    lua_pushvalue(L, 1);
    lua_pushnil(L);
    return 3;
}

static const struct luaL_Reg luavvd_objlist [] = {
    {"__newindex", objlist_newindex},
    {"__index",    objlist_getindex},
    {"__len",      objlist_getlen},
    {"__pairs",    objlist_getpairs},
    {"__ipairs",   objlist_getpairs},
    {NULL, NULL} /* sentinel */
};

/* PUBLIC FUNCTIONS */
/* vvvvvvvvvvvvvvvv */

int luaopen_objlist (lua_State *L) {
    luaL_newmetatable(L, "ObjList"); // push 1
    lua_pushstring(L, "ObjList"); // push 2
    lua_setfield(L, -2, "__name"); // pop 2
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
