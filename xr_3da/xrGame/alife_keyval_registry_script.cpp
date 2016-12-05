////////////////////////////////////////////////////////////////////////////
//	Module 		: alife_keyval_registry_script.cpp
//	Created 	: 03.12.2016
//  Modified 	: 03.12.2016
//	Author		: jarni
//	Description : ALife key-value pairs registry
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
