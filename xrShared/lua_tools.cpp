#include "stdafx.h"
#include "lua_tools.h"

XRSHARED_EXPORT lua_State* g_game_lua=nullptr;	
XRSHARED_EXPORT lua_State* g_active_lua=nullptr;	

XRSHARED_EXPORT LPCSTR get_lua_traceback(lua_State *L, int depth)
{
	if (L)  g_active_lua = L;
	if (!L) L = g_active_lua;
	if (!L) return "Lua STATE = NULL";

	static char  buffer[32768]; 
	Memory.mem_fill(buffer, 0, sizeof(buffer));
	int top = lua_gettop(L);
	// alpet: Lua traceback added
	lua_getfield(L, LUA_GLOBALSINDEX, "debug");
	lua_getfield(L, -1, "traceback");
	lua_pushstring(L, "\t");
	lua_pushinteger(L, 1);

	const char *traceback = "cannot get Lua traceback ";
	strcpy_s(buffer, 32767, traceback);
	__try
	{
		if (0 == lua_pcall(L, 2, 1, 0))
		{
			traceback = lua_tostring(L, -1);
			strcpy_s(buffer, 32767, traceback);
			lua_pop(L, 1);
		}
	}
	__except (EXCEPTION_EXECUTE_HANDLER)
	{
		Msg("!#EXCEPTION(get_lua_traceback): buffer = %s ", buffer);
	}
	lua_settop (L, top);
	return buffer;
}