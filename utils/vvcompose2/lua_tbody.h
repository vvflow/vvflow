#include "core.h"

int luaopen_tbody (lua_State *L);

/* push userdata with (TBody*) and set metatable "LibVVD.TBody" */
void  pushTBody(lua_State *L, TBody* body);
/* get userdata (TBody*) and check metatable is "LibVVD.TBody" */
TBody* checkTBody(lua_State *L, int idx);

// int luavvd_setTBody(lua_State *L);
// int luavvd_getTBody(lua_State *L);
int luavvd_load_body(lua_State *L);

// for gen_*()
int luavvd_gen_cylinder(lua_State *L);
int luavvd_gen_plate(lua_State *L);
void gen_line(std::vector<TAtt>& alist, TVec p1, TVec p2, size_t N, uint32_t slip = 0);
void gen_arc(std::vector<TAtt>& alist, TVec c, double R, double a1, double a2, size_t N, uint32_t slip = 0);
