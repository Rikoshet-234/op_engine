////////////////////////////////////////////////////////////////////////////
//	Module 		: saved_game_wrapper_script.cpp
//	Created 	: 21.02.2006
//  Modified 	: 21.02.2006
//	Author		: Dmitriy Iassenev
//	Description : saved game wrapper class script export
////////////////////////////////////////////////////////////////////////////

#include "pch_script.h"
#include "alife_keyval_registry.h"
#include "alife_simulator.h"
#include "ai_space.h"

using namespace luabind;

static CALifeKeyvalRegistry *keyvals()
{
	return			(const_cast<CALifeKeyvalRegistry*>(&ai().get_alife()->keyvals()));
}

//#pragma optimize("s",on)
void CALifeKeyvalRegistry::script_register(lua_State *L)
{
	module(L)
	[
		class_<CALifeKeyvalRegistry>("CALifeKeyvalRegistry")
			//.def(constructor<>())
			.def("get", &CALifeKeyvalRegistry::get)
			.def("set", &CALifeKeyvalRegistry::set)
		
		,def("keyvals", &keyvals)
	];
}
