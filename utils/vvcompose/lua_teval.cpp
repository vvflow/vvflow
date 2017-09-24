#include "lua_teval.h"
#include "getset.h"

// #include <stdio.h>
#include <cstring>
#include <stdexcept>

static int teval_eval(lua_State *L) {
    TEval* script = checkTEval(L, 1);
    lua_Number val = luaL_checknumber(L, 2);

    lua_pushnumber(L, script->eval(val));
    return 1;
}

static int teval_tostring(lua_State *L) {
    TEval* script = checkTEval(L, 1);
    lua_pushstring(L, std::string(*script).c_str());
    return 1;
}

static const struct luaL_Reg teval_methods[] = {
    {"eval",     teval_eval},
    {"tostring", teval_tostring},
    {NULL, NULL}
};

static int teval_newindex(lua_State *L) {
    return luaL_error(L, "TEval has no parameters");
}

static const struct luaL_Reg luavvd_teval [] = {
    {"__tostring", teval_tostring},
    {"__newindex", teval_newindex},
    {NULL, NULL} /* sentinel */
};

/* PUBLIC FUNCTIONS */
/* vvvvvvvvvvvvvvvv */

int luaopen_teval (lua_State *L) {
    luaL_newmetatable(L, "TEval"); // push 1
    luaL_newlib(L, teval_methods); // push 2
    lua_setfield(L, -2, "__index");      // pop 2
    luaL_setfuncs(L, luavvd_teval, 0); /* register metamethods */
    return 0;
}

void  pushTEval(lua_State *L, TEval* script) {
    TEval** udat = (TEval**)lua_newuserdata(L, sizeof(TEval*));
    *udat = script;
    luaL_getmetatable(L, "TEval");
    lua_setmetatable(L, -2);
}

TEval* checkTEval(lua_State *L, int idx) {
    TEval** udat = (TEval**)luaL_checkudata(L, idx, "TEval");
    return *udat;
}

int luavvd_setTEval(lua_State *L) {
    TEval* script = (TEval*)lua_touserdata(L, 1);
    std::string str;

    switch (lua_type(L, 2)) {
    case LUA_TSTRING:
        str = lua_tostring(L, 2);
        break;
    case LUA_TUSERDATA: {
        TEval** udata = (TEval**)luaL_testudata(L, 2, "TEval");

        if (!udata) {
            const char *typ;
            if (luaL_getmetafield(L, 2, "__name") == LUA_TSTRING)
                typ = lua_tostring(L, -1);
            else
                typ = "userdata";
            lua_pushfstring(L, "string or TEval expected, got %s", typ);
            return 1;
        }

        str = **udata;
        break;
    }
    default:
        const char *typ = luaL_typename(L, 2);
        lua_pushfstring(L, "string or TEval expected, got %s", typ);
        return 1;
    }

    try {
        *script = str;
        return 0;
    } catch (const std::invalid_argument& e) {
        lua_pushfstring(L, "invalid expression '%s'", str.c_str());
        return 1;
    }
}

int luavvd_getTEval(lua_State *L) {
    TEval* script = (TEval*)lua_touserdata(L, 1);    
    pushTEval(L, script);
    return 1;
}
