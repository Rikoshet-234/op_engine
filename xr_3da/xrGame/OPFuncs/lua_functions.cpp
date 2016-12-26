#include "stdafx.h"
#include "lua_functions.h"

#include <luabind/luabind.hpp>
#include "../xr_3da/xrGame/inventory_space.h"
#include "../xrCore/OPFuncs/op_engine_version.h"
#include "../ai_space.h"
#include "../script_engine.h"
#include "../script_game_object.h"
#include "../ui/UIInventoryUtilities.h"


namespace OPFuncs
{
	bool luaFunctionExist(lua_State *L,std::string functionPath)
	{
		int save_top = lua_gettop(L);
		std::string name_space = functionPath.substr(0,functionPath.find("."));
		std::string func_name  = functionPath.substr(name_space.length());
		if (!func_name.empty())
		{
			lua_getglobal(L, name_space.c_str());
			if (lua_istable(L, -1))
				lua_getfield(L, -1, func_name.c_str());
			else
			{
				lua_settop(L, save_top);
				return false;
			}
		}
		else
		{
			func_name = name_space;
			name_space = "_G";
			lua_getglobal(L, func_name.c_str()); 
		}
		return !lua_isfunction(L, -1);

	}

	u32	get_actor_slots_count()
	{
		return SLOTS_TOTAL; //from inventory_space.h
	}	

	LPCSTR lua_GetOPEngineVersion()
	{
		return GetOPEngineVersion();
	}

	bool lua_IsOPEngine()
	{
		return true;

	}


	void registerLuaOPFuncs(lua_State *L)
	{
		luabind::module(L)
			[
				luabind::def("get_actor_slots_count",&get_actor_slots_count),
				luabind::def("IsOPEngine", &lua_IsOPEngine),
				luabind::def("engine_log", static_cast<void(*)(LPCSTR)>(&Log)),
				luabind::def("GetOPEngineVersion",&lua_GetOPEngineVersion)
			];
	}
}
 