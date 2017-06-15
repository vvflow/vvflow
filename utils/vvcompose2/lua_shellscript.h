#include "core.h"

int luaopen_shellscript (lua_State *L);

/* push userdata with (ShellScript*) and set metatable "LibVVD.ShellScript" */
void  pushShellScript(lua_State *L, ShellScript* script);
/* get userdata (ShellScript*) and check metatable is "LibVVD.ShellScript" */
ShellScript* checkShellScript(lua_State *L, int idx);

int luavvd_setShellScript(lua_State *L);
int luavvd_getShellScript(lua_State *L);
