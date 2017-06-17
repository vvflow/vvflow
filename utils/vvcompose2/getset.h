#include <lua.hpp>
// #include 

/* table.name = *ptr = value 
    1: ptr
    2: name
    3: value
*/
int luavvd_setdouble(lua_State *L);
int luavvd_setint32(lua_State *L);
int luavvd_setstring(lua_State *L);
int luavvd_setTTime(lua_State *L);

/* return *ptr
    1: ptr
*/
int luavvd_getdouble(lua_State *L);
int luavvd_getint32(lua_State *L);
int luavvd_getstring(lua_State *L);
int luavvd_getTTime(lua_State *L);

struct luavvd_member {
    const char *name;
    lua_CFunction getter;
    lua_CFunction setter;
    size_t offset;
};

struct luavvd_method {
    const char *name;
    lua_CFunction func;
};

// const char* luavvd_typename(lua_State *L, int idx);