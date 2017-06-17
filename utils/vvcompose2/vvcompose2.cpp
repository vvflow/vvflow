#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <lua.hpp>

#include "core.h"
#include "getset.h"
#include "lua_tvec.h"
#include "lua_tvec3d.h"
#include "lua_tbody.h"
#include "lua_shellscript.h"

static Space* S;

int stackDump(lua_State *L)
{
    int i;
    int top = lua_gettop(L); /* depth of the stack */
    for (i = 1; i <= top; i++) { /* repeat for each level */
        int t = lua_type(L, i);
        switch (t) {
        case LUA_TSTRING: { /* strings */
            printf("%d str  '%s'\n", i, lua_tostring(L, i));
            break;
        }
        case LUA_TBOOLEAN: { /* Booleans */
            printf("%d bool %s\n", i, lua_toboolean(L, i) ? "true" : "false");
            break;
        }
        case LUA_TNUMBER: { /* numbers */
            printf("%d num  %g\n", i, lua_tonumber(L, i));
            break;
        }
        default: { /* other values */
            printf("%d %s\n", i, lua_typename(L, t));
            break;
        }
        }
    }
    printf("\n"); /* end the listing */
}

int space_load(lua_State *L) {
    void* ptr = luaL_checkudata(L, 1, "LuaVVD");
    const char* fname = lua_checkstring(L, 2);
    S->Load(fname);
    return 0;
}

int space_save(lua_State *L) {
    void* ptr = luaL_checkudata(L, 1, "LuaVVD");
    const char* fname = lua_checkstring(L, 2);
    S->Save(fname);
    return 0;
}

static const struct luavvd_member space_members[] = {
    {"caption", luavvd_getstring,      luavvd_setstring,      offsetof(Space, caption) },
    {"re",      luavvd_getdouble,      luavvd_setdouble,      offsetof(Space, Re) },
    {"finish",  luavvd_getdouble,      luavvd_setdouble,      offsetof(Space, Finish) },
    {"inf_g",   luavvd_getdouble,      luavvd_setdouble,      offsetof(Space, InfCirculation) },
    {"inf_vx",  luavvd_getShellScript, luavvd_setShellScript, offsetof(Space, InfSpeedX) },
    {"inf_vy",  luavvd_getShellScript, luavvd_setShellScript, offsetof(Space, InfSpeedY) },
    {"gravity", luavvd_getTVec,        luavvd_setTVec,        offsetof(Space, gravitation) },

    {"time",       luavvd_getTTime, luavvd_setTTime, offsetof(Space, Time) },
    {"dt",         luavvd_getTTime, luavvd_setTTime, offsetof(Space, dt) },
    {"dt_save",    luavvd_getTTime, luavvd_setTTime, offsetof(Space, dt_save) },
    {"dt_streak",  luavvd_getTTime, luavvd_setTTime, offsetof(Space, dt_streak) },
    {"dt_profile", luavvd_getTTime, luavvd_setTTime, offsetof(Space, dt_profile) },

    {NULL, NULL, NULL, 0} /* sentinel */    
};

static const struct luavvd_method space_methods[] = {
    {"load", space_load},
    {"save", space_save},
    {NULL, NULL}
} ;

static int luavvd_newindex(lua_State *L) {
    // static Space *S;
    const char *name = luaL_checkstring(L, 2);

    for (auto f = space_members; f->name; f++) {
        if (strcmp(name, f->name)) continue;
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

static int luavvd_getindex(lua_State *L) {
    // static Space *S;
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

    return luaL_error(L, "S has no member '%s'", name);
}

int luavvd_gen_cylinder(lua_State *L);
static const struct luaL_Reg luavvd [] = {
    // gen_functions
    // {"gen_cylinder", luavvd_gen_cylinder},
    // {"newTVec", newTVec},
    // {"__newindex", stackDump},
    {NULL, NULL} /* sentinel */
};

extern "C" {
    int luaopen_vvd (lua_State *L);
}

int luaopen_vvd (lua_State *L) {
    // lua_pushvalue(L, -1); /* duplicate the metatable */
    // lua_setfield(L, -2, "__index"); /* mt.__index = mt */

    // luaL_setfuncs(L, luavvd, 0); /* register metamethods */
    S = new Space();
    luaL_newlib(L, luavvd); // push 1
    
    luaL_newmetatable(L, "LuaVVD"); // push 2
    lua_pushcfunction(L, luavvd_newindex); // push 3
    lua_setfield(L, -2, "__newindex"); // pop 3
    lua_pushcfunction(L, luavvd_getindex); // push 3
    lua_setfield(L, -2, "__index"); // pop 3
    
    lua_setmetatable(L, -2); // pop 2
    lua_setglobal(L, "S"); // pop 1

    lua_pushcfunction(L, luavvd_gen_cylinder); // push 1
    lua_setglobal(L, "gen_cylinder"); // pop 1

    luaopen_tvec(L);
    luaopen_tvec3d(L);
    luaopen_tbody(L);
    luaopen_shellscript(L);

    return 0;
}


int main (int argc, char** argv) {
    if (argc < 2) {
        fprintf(stderr, "Missing filename\n");
        exit(-1);
    } else if (argc > 2) {
        fprintf(stderr, "Too many arguments\n");
        exit(-1);
    }

    lua_State *L = luaL_newstate();   /* opens Lua */
    luaL_openlibs(L);            /* opens the basic library */
    luaopen_vvd(L);            /* opens the basic library */

    if (luaL_dofile(L, argv[1])) {
        fprintf(stderr, "%s\n", lua_tostring(L, -1));
        lua_pop(L, 1);
    }

    lua_close(L);
    return 0;
}