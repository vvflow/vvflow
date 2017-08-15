#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <getopt.h>
#include <signal.h>
#include <lua.hpp>

#include "core.h"
#include "getset.h"
#include "lua_tvec.h"
#include "lua_tobj.h"
#include "lua_tvec3d.h"
#include "lua_tbody.h"
#include "lua_shellscript.h"
#include "lua_objlist.h"
#include "lua_bodylist.h"
#include "gen_body.h"

#include "linenoise.h"

int stackDump(lua_State *L)
{
    int i;
    int top = lua_gettop(L); /* depth of the stack */
    for (i = 1; i <= top; i++) { /* repeat for each level */
        int t = lua_type(L, i);
        switch (t) {
        case LUA_TSTRING: { /* strings */
            printf("#%d str  '%s'\n", i, lua_tostring(L, i));
            break;
        }
        case LUA_TBOOLEAN: { /* Booleans */
            printf("#%d bool %s\n", i, lua_toboolean(L, i) ? "true" : "false");
            break;
        }
        case LUA_TNUMBER: { /* numbers */
            printf("#%d num  %g\n", i, lua_tonumber(L, i));
            break;
        }
        case LUA_TUSERDATA: {
            const char *typ;
            if (luaL_getmetafield(L, i, "__name")) {
                typ = lua_tostring(L, -1);
                lua_pop(L, 1);
            }
            else
                typ = "userdata";
            printf("#%d udata %s\n", i, typ);
            break;
        }
        default: { /* other values */
            printf("#%d %s\n", i, lua_typename(L, t));
            break;
        }
        }
    }
    printf("\n"); /* end the listing */
}

int space_load(lua_State *L) {
    Space **ptr = (Space**)luaL_checkudata(L, 1, "S");
    Space *S = *ptr;
    const char* fname = luaL_checkstring(L, 2);
    S->Load(fname);
    return 0;
}

int space_save(lua_State *L) {
    Space **ptr = (Space**)luaL_checkudata(L, 1, "S");
    Space *S = *ptr;
    const char* fname = luaL_checkstring(L, 2);
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

    {"body_list",         luavvd_getBodyList, NULL, offsetof(Space, BodyList) },
    {"vort_list",          luavvd_getObjList, NULL, offsetof(Space, VortexList) },
    {"sink_list",          luavvd_getObjList, NULL, offsetof(Space, SourceList) },
    {"streak_source_list", luavvd_getObjList, NULL, offsetof(Space, StreakSourceList) },
    {"streak_domain_list", luavvd_getObjList, NULL, offsetof(Space, StreakList) },

    {NULL, NULL, NULL, 0} /* sentinel */    
};

static const struct luavvd_method space_methods[] = {
    {"load", space_load},
    {"save", space_save},
    {NULL, NULL}
} ;

static int luavvd_newindex(lua_State *L) {
    Space **ptr = (Space**)luaL_checkudata(L, 1, "S");
    Space *S = *ptr;
    const char *name = luaL_checkstring(L, 2);

    for (auto f = space_members; f->name; f++) {
        if (strcmp(name, f->name)) continue;
        if (!f->setter) break;
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
    Space **ptr = (Space**)luaL_checkudata(L, 1, "S");
    Space *S = *ptr;
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

    lua_pushnil(L);
    return 1;
}


static const struct luaL_Reg luavvd_space [] = {
    {"__newindex", luavvd_newindex},
    {"__index",    luavvd_getindex},
    {NULL, NULL} /* sentinel */
};

extern "C" {
    int luaopen_vvd (lua_State *L);
}

int luaopen_vvd (lua_State *L) {
    static Space S;
    Space** ptr = (Space**)lua_newuserdata(L, sizeof(Space*)); // push 1 (Space)
    *ptr = &S;
    luaL_newmetatable(L, "S"); // push 2 (Space.mt)
    luaL_setfuncs(L, luavvd_space, 0);
    lua_setmetatable(L, -2); // pop 2
    lua_setglobal(L, "S"); // pop 1

    lua_pushnumber(L, std::numeric_limits<double>::infinity()); // push 1
    lua_setglobal(L, "inf"); // pop 1
    lua_pushnumber(L, std::numeric_limits<double>::quiet_NaN()); // push 1
    lua_setglobal(L, "nan"); // pop 1

    lua_pushcfunction(L, luavvd_load_body); // push 1
    lua_setglobal(L, "load_body"); // pop 1
    lua_pushcfunction(L, luavvd_gen_cylinder); // push 1
    lua_setglobal(L, "gen_cylinder"); // pop 1
    lua_pushcfunction(L, luavvd_gen_plate); // push 1
    lua_setglobal(L, "gen_plate"); // pop 1
    lua_pushcfunction(L, luavvd_gen_gis); // push 1
    lua_setglobal(L, "gen_gis"); // pop 1

    luaopen_tvec(L);
    luaopen_tobj(L);
    luaopen_tvec3d(L);
    luaopen_tbody(L);
    luaopen_objlist(L);
    luaopen_bodylist(L);
    luaopen_shellscript(L);

    return 0;
}

volatile sig_atomic_t c_sigint = 0;
void handle_sigint(int signum) {
    c_sigint = 1;
    printf("caught sigint\n");
    return;
}

int main (int argc, char** argv) {

    int c;

    while ( (c = getopt(argc, argv, "hv")) != -1 ) {
        switch (c) {
        case 'h':
            return execlp("man", "man", "vvcompose", NULL);
        case 'v':
            printf("vvcompose %s\n", libvvhd_gitinfo);
            printf("revision: %s\n", libvvhd_gitrev);
            if (libvvhd_gitdiff[0] != '\0')
                printf("git_diff: %s\n", libvvhd_gitdiff);
            return 0;
        case '?':
            return 1;
        default:
            abort();
        }
    }

    lua_State *L = luaL_newstate();   /* opens Lua */
    luaL_openlibs(L);          /* opens the basic library */
    luaopen_vvd(L);
    int ret = 0;

    if (argc < 2) {
        /* interpreter mode */
        signal(SIGINT, handle_sigint);

        char *line;
        while(true) {
            line = linenoise("vvcompose> ");
            if (errno == EAGAIN) {
                errno = 0;
                continue;
            } else if (!line) {
                break;
            }

            linenoiseHistoryAdd(line);

            int err = luaL_loadstring(L, line) || lua_pcall(L, 0, 0, 0);
            if (err) {
                printf("%s\n", lua_tostring(L, -1));
                lua_pop(L, 1);
            }
            
            linenoiseFree(line);
        }
    } else {
        lua_newtable(L);
        for (int i=optind+1; i<argc; i++) {
            lua_pushstring(L, argv[i]);
            lua_rawseti(L, -2, i-optind);
        }
        lua_setglobal(L, "arg");

        ret = luaL_dofile(L, argv[optind]);

        if (ret) {
            fprintf(stderr, "%s\n", lua_tostring(L, -1));
            lua_pop(L, 1);
        }
    }

    lua_close(L);
    return ret;
}
