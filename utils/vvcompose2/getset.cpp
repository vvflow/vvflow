#include <lua.hpp>
#include "getset.h"

int luavvd_setdouble(lua_State *L) {
    double* dbl = (double*)lua_touserdata(L, 1);
    
    int isnum;
    lua_Number val = lua_tonumberx(L, 2, &isnum);
    if (!isnum) {
        lua_pushfstring(L, "number expected, got %s", luaL_typename(L, 2));
        return 1;
    }

    *dbl = val;
    return 0;
}

// void luavvd_setdouble(lua_State *L, void* ptr) {
//     double* dbl = (double*)ptr;
//     int isnum;
    
//     const char *key = lua_tostring(L, -2);
//     lua_Number val = lua_tonumberx(L, -1, &isnum);
//     if (!isnum) {
//         const char *typ = luaL_typename(L, -1);
//         luaL_error(L, "bad argument to '%s' (number expected, got %s)", key, typ);
//     }
//     *dbl = val;
// }

int luavvd_getdouble(lua_State *L) {
    double* dbl = (double*)lua_touserdata(L, 1);    
    lua_pushnumber(L, *dbl);
    return 1;
}

// void luavvd_getdouble(lua_State *L, void* ptr) {
//     double* dbl = (double*)ptr;
//     lua_pushnumber(L, *dbl);
// }

// const char* luavvd_typename(lua_State *L, int idx) {
//     int typ = lua_type(L, idx);
//     if (typ == LUA_TUSERDATA) {
//         lua_getmetatable(L, idx);
//         if (lua_getfield(L, -1, "__typename") == LUA_TSTRING) {
//             return lua_tostring(L, -1);
//         }
//     }
    
//     return lua_typename(L, typ);
// }
