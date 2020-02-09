#include "lua_bodylist.h"
#include "lua_tbody.h"
#include "getset.h"

// #include <cstdio>
#include <cstring>

using std::vector;
using std::shared_ptr;

static int bodylist_insert(lua_State *L) {
    vector<shared_ptr<TBody>>& li = *checkBodyList(L, 1);
    shared_ptr<TBody> body = checkTBody(L, 2);
    for (auto lb = li.begin(); lb != li.end(); lb++) {
        if (lb->get() == body.get()) {
            body.reset(); // to prevent lua_error spoiling reference counting
            return luaL_argerror(L, 2, "can not insert the body twice");
        }
    }

    li.push_back(body);
    return 0;
}

static int bodylist_erase(lua_State *L) {
    vector<shared_ptr<TBody>>& li = *checkBodyList(L, 1);
    shared_ptr<TBody> body = checkTBody(L, 2);
    for (auto lb = li.begin(); lb != li.end(); lb++) {
        if (lb->get() == body.get()) {
            lb = li.erase(lb);
            return 0;
        }
    }
    body.reset(); // to prevent lua_error spoiling reference counting
    return luaL_argerror(L, 2, "the body is not in list");
}

static int bodylist_clear(lua_State *L) {
    vector<shared_ptr<TBody>>& li = *checkBodyList(L, 1);
    li.clear();
    return 0;
}

static int bodylist_getlen(lua_State *L) {
    vector<shared_ptr<TBody>> *li = checkBodyList(L, 1);
    lua_pushinteger(L, li->size());
    return 1;
}

static const struct luavvd_method bodylist_methods[] = {
    {"insert",   bodylist_insert},
    {"erase",    bodylist_erase},
    {"clear",    bodylist_clear},

    {NULL, NULL}
};

static int bodylist_newindex(lua_State *L) {
    return luaL_error(L, "TBodyList can not assign anything");
}

static int bodylist_getindex(lua_State *L) {
    vector<shared_ptr<TBody>>* li = checkBodyList(L, 1);

    if (lua_isnumber(L, 2)) {
        lua_Integer idx = lua_tointeger(L, 2)-1;
        if (idx >= 0 && idx < li->size()) {
            pushTBody(L, li->at(idx));
            return 1;
        }
    } else if (lua_isstring(L, 2)) {
        const char *name = luaL_checkstring(L, 2);
        for (auto f = bodylist_methods; f->name; f++) {
            if (strcmp(name, f->name)) continue;
            lua_pushcfunction(L, f->func);
            return 1;
        }
    }

    lua_pushnil(L);
    return 1;
}

static int bodylist_getnext(lua_State *L) {
    vector<shared_ptr<TBody>>& li = *checkBodyList(L, 1);

    int idx = luaL_optinteger(L, 2, 0);
    if (idx >= li.size()) {
        lua_pushnil(L);
        return 1;
    } else {
        lua_pushinteger(L, idx+1);
        pushTBody(L, li.at(idx));
        return 2;
    }
}

static int bodylist_getipairs(lua_State *L) {
    lua_pushcfunction(L, bodylist_getnext);
    lua_pushvalue(L, 1);
    lua_pushnil(L);
    return 3;
}

static const struct luaL_Reg luavvd_bodylist [] = {
    {"__newindex", bodylist_newindex},
    {"__index",    bodylist_getindex},
    {"__len",      bodylist_getlen},
    {"__ipairs",   bodylist_getipairs},
    {NULL, NULL} /* sentinel */
};

/* PUBLIC FUNCTIONS */
/* vvvvvvvvvvvvvvvv */

int luaopen_bodylist (lua_State *L) {
    luaL_newmetatable(L, "BodyList"); // push 1
    lua_pushstring(L, "BodyList"); // push 2
    lua_setfield(L, -2, "__name"); // pop 2
    luaL_setfuncs(L, luavvd_bodylist, 0); /* register metamethods */
    return 0;
}

void  pushBodyList(lua_State *L, vector<shared_ptr<TBody>>* li) {
    vector<shared_ptr<TBody>>** udat =
        (vector<shared_ptr<TBody>>**)lua_newuserdata(L, sizeof(void*));
    *udat = li;
    luaL_getmetatable(L, "BodyList");
    lua_setmetatable(L, -2);
}

vector<shared_ptr<TBody>>* checkBodyList(lua_State *L, int idx) {
    vector<shared_ptr<TBody>>** udat =
        (vector<shared_ptr<TBody>>**)luaL_checkudata(L, idx, "BodyList");
    return *udat;
}

int luavvd_getBodyList(lua_State *L) {
    vector<shared_ptr<TBody>>* li = (vector<shared_ptr<TBody>>*)lua_touserdata(L, 1);
    pushBodyList(L, li);
    return 1;
}
