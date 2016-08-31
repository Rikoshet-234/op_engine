#include "stdafx.h"
#include "lua_functions.h"

#include <luabind/luabind.hpp>
#include "../xr_3da/xrGame/inventory_space.h"
#include "../xrCore/OPFuncs/op_engine_version.h"



namespace OPFuncs
{
	u32	get_actor_slots_count()
	{
		return SLOTS_TOTAL-1; //from inventory_space.h
	}	

	LPCSTR lua_GetOPEngineVersion()
	{
		std::string version=GetOPEngineVersion();
		char* result=new char[version.length()+1];
		strcpy(result,version.c_str());
		return result;
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
 