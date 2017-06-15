#include <lua.hpp>
#include <stdio.h>
#include <string.h>

#include "getset.h"
#include "lua_shellscript.h"

static int shellscript_eval(lua_State *L) {
    ShellScript* script = checkShellScript(L, 1);
    lua_Number val = luaL_checknumber(L, 2);

    int isnum;
    val = lua_tonumberx(L, 2, &isnum);
    if (!isnum) {
        luaL_error(L, "number expected, got %s", luaL_typename(L, 2));
        return 1;
    }

    lua_pushnumber(L, script->getValue(val));
    return 1;
}

static int shellscript_tostring(lua_State *L) {
    ShellScript* script = checkShellScript(L, 1);
    lua_pushstring(L, std::string(*script).c_str());
    return 1;
}

static const struct luaL_Reg shellscript_methods[] = {
    {"eval",    shellscript_eval},
    {"2string", shellscript_tostring},
    {NULL, NULL}
};

static int shellscript_newindex(lua_State *L) {
    luaL_error(L, "ShellScript has no parameters");
    return 0;
}

static const struct luaL_Reg luavvd_shellscript [] = {
    {"__tostring", shellscript_tostring},
    {"__newindex", shellscript_newindex},
    {NULL, NULL} /* sentinel */
};

/* PUBLIC FUNCTIONS */
/* vvvvvvvvvvvvvvvv */

int luaopen_shellscript (lua_State *L) {
    luaL_newmetatable(L, "ShellScript"); // push 1
    luaL_newlib(L, shellscript_methods); // push 2
    lua_setfield(L, -2, "__index");      // pop 2
    luaL_setfuncs(L, luavvd_shellscript, 0); /* register metamethods */
    return 0;
}

void  pushShellScript(lua_State *L, ShellScript* script) {
    ShellScript** udat = (ShellScript**)lua_newuserdata(L, sizeof(ShellScript*));
    *udat = script;
    luaL_getmetatable(L, "ShellScript");
    lua_setmetatable(L, -2);
}

ShellScript* checkShellScript(lua_State *L, int idx) {
    ShellScript** udat = (ShellScript**)luaL_checkudata(L, idx, "ShellScript");
    return *udat;
}

int luavvd_setShellScript(lua_State *L) {
    ShellScript* script = (ShellScript*)lua_touserdata(L, 1);
    
    switch (lua_type(L, 2)) {
    case LUA_TSTRING: {
        const char* s = lua_tostring(L, 2);
        if (!script->setEvaluator(s)) {
            lua_pushfstring(L, "invalid expression '%s'", s);
            return 1;
        }
        return 0;
    }
    case LUA_TUSERDATA: {
        ShellScript** udata = (ShellScript**)luaL_testudata(L, 2, "ShellScript");

        if (!udata) {
            const char *typ;
            if (luaL_getmetafield(L, 2, "__name") == LUA_TSTRING)
                typ = lua_tostring(L, -1);
            else
                typ = "userdata";
            lua_pushfstring(L, "string or ShellScript expected, got %s", typ);
            return 1;
        }

        script->setEvaluator(std::string(**udata).c_str());
        return 0;
    }
    }
    
    const char *typ = luaL_typename(L, 2);
    lua_pushfstring(L, "string or ShellScript expected, got %s", typ);
    return 1;
}

int luavvd_getShellScript(lua_State *L) {
    ShellScript* script = (ShellScript*)lua_touserdata(L, 1);    
    pushShellScript(L, script);
}
