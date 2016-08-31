#ifndef lua_toolsH
#define lua_toolsH

#include "xrShared.h"

XRSHARED_EXPORT LPCSTR get_lua_traceback(lua_State *L, int depth);
XRSHARED_EXPORT extern lua_State* g_game_lua;	
XRSHARED_EXPORT extern lua_State* g_active_lua;	

#endif