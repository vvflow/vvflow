#include <lua.hpp>

/* class.member = *ptr = value
pop
    1: ptr (lightuserdata)
    2: value
push
    1: nil or error
*/
int luavvd_setdouble(lua_State *L); // (double*)
int luavvd_setint32(lua_State *L);  // (int32_t*)
int luavvd_setstring(lua_State *L); // (std::string*)
int luavvd_setTTime(lua_State *L);  // (TTime*)
int luavvd_setTVec(lua_State *L);   // (TVec*)
int luavvd_setTVec3D(lua_State *L); // (TVec3D*)
int luavvd_setShellScript(lua_State *L); // (ShellScript*)
/* return *ptr
pop
    1: ptr (lightuserdata)
push
    1: value
*/
int luavvd_getdouble(lua_State *L); // lua_pushnumber
int luavvd_getint32(lua_State *L);  // lua_pushinteger
int luavvd_getstring(lua_State *L); // lua_pushstring
int luavvd_getTTime(lua_State *L);  // lua_pushnumber
int luavvd_getTVec(lua_State *L);   // lua_newuserdata(TVec*, "TVec")
int luavvd_getTVec3D(lua_State *L); // lua_newuserdata(TVec3D*, "TVec3D")
int luavvd_getShellScript(lua_State *L); // lua_newuserdata(ShellScript*, "ShellScript")
int luavvd_getObjList(lua_State *L); // lua_newuserdata(std::vector<TObj>*, "ObjList")

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

// template<typename T, const char* meta>
// void  lua_pushptr(lua_State *L, T* vec);