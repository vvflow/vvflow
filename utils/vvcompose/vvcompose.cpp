#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <lua.hpp>

#include "core.h"
#include "getset.h"

#include "lua_space.h"
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

int luaopen_vvd (lua_State *L) {

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

    luaopen_space(L);
    luaopen_tvec(L);
    luaopen_tobj(L);
    luaopen_tvec3d(L);
    luaopen_tbody(L);
    luaopen_objlist(L);
    luaopen_bodylist(L);
    luaopen_shellscript(L);

    return 0;
}

int main (int argc, char** argv) {

    if (argc > 1) {
        // try to parse arguments
        if (!strcmp(argv[1], "-h") || !strcmp(argv[1], "--help")) {
            return execlp("man", "man", "vvcompose", NULL);
        } else if (!strcmp(argv[1], "-v") || !strcmp(argv[1], "--version")) {
            fprintf(stderr, "vvcompose %s\n", libvvhd_gitinfo);
            fprintf(stderr, "revision: %s\n", libvvhd_gitrev);
            if (libvvhd_gitdiff[0] == '\0') return 0;
            fprintf(stderr, "git_diff: %s\n", libvvhd_gitdiff);
            return 0;
        } else if (!strcmp(argv[1], "--")) {
            argv[1] = argv[0];
            argc--;
            argv++;
        } else if (!strncmp(argv[1], "-", 1)) {
            fprintf(stderr, "%s: invalid option '%s'\n", argv[0], argv[1]);
            return 1;
        }
    }

    lua_State *L = luaL_newstate();
    luaL_openlibs(L);
    luaopen_vvd(L);
    int ret = 0;

    if (argc > 1) {
        /* load script from file */
        lua_newtable(L);
        for (int i=1; i<argc; i++) {
            lua_pushstring(L, argv[i]);
            lua_rawseti(L, -2, i-1);
        }
        lua_setglobal(L, "arg");

        ret = luaL_dofile(L, argv[1]);

        if (ret) {
            fprintf(stderr, "%s\n", lua_tostring(L, -1));
            lua_pop(L, 1);
        }
    } else {
        /* interpreter mode */
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
    }

    lua_close(L);
    return ret;
}
