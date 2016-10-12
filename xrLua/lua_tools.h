#ifndef lua_toolsH
#define lua_toolsH

#include "luaconf.h"

extern "C" {
	#include <lua.h>
	#include <luajit.h>
	#include <lcoco.h>
};

#ifdef LUA_BUILD_AS_DLL
	#define LUATOOLS_EXPORT __declspec(dllexport)
#else
	#define LUATOOLS_EXPORT __declspec(dllimport)
	#pragma comment(lib,"xrLua")
#endif 


LUATOOLS_EXPORT LPCSTR get_lua_traceback(lua_State *L, int depth);
LUATOOLS_EXPORT extern lua_State* g_game_lua;	
LUATOOLS_EXPORT extern lua_State* g_active_lua;	

#endif