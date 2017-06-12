#include <lua.hpp>
#include "getset.h"
#include "core.h"

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

int luavvd_getdouble(lua_State *L) {
    double* dbl = (double*)lua_touserdata(L, 1);    
    lua_pushnumber(L, *dbl);
    return 1;
}


int luavvd_setTTime(lua_State *L) {
    TTime* t = (TTime*)lua_touserdata(L, 1);

    printf("Set ttime\n");
    int isnum[2];
    switch (lua_type(L, 2)) {
    case LUA_TNUMBER:
        *t = TTime::makeWithSecondsDecimal(lua_tonumber(L, 2));
        return 0;
    case LUA_TTABLE:
        for (size_t rawlen = lua_rawlen(L, 2); rawlen != 2; ) {
            lua_pushfstring(L, "TTime needs table with two elements, got %d", rawlen);
            return 1;
        }

        lua_geti(L, 2, 1); // push table[1]
        lua_geti(L, 2, 2); // push table[2]        
        t->value = lua_tointegerx(L, -2, &isnum[0]);
        t->timescale = lua_tointegerx(L, -1, &isnum[1]);
        if (!isnum[0] || !isnum[1]) {
            const char *typ1 = luaL_typename(L, -2);
            const char *typ2 = luaL_typename(L, -1);
            lua_pushfstring(L, "TTime needs two integers, got %s and %s", typ1, typ2);
            return 1;
        } else if (lua_tointegerx(L, -1)<1) {
            lua_pushfstring(L, "TTime timescale must be unsigned, got %d", typ1, typ2);
            return 1;
        }

        return 0;
    }
    
    const char *typ = luaL_typename(L, 2);
    lua_pushfstring(L, "table or number expected, got %s", typ);
    return 1;
}

int luavvd_getTTime(lua_State *L) {
    TTime* t = (TTime*)lua_touserdata(L, 1);    
    lua_pushnumber(L, double(*t));
    return 1;
}
