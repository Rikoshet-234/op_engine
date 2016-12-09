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
#include "luabind/luabind.hpp"
#include "luabind/iterator_policy.hpp"

using namespace luabind;

static CALifeKeyvalContainer *keyvals()
{
	return (const_cast<CALifeKeyvalRegistry*>(&ai().get_alife()->keyvals())->container(nullptr));
}

static CALifeKeyvalContainer *keyvals(LPCSTR name)
{
	return (const_cast<CALifeKeyvalRegistry*>(&ai().get_alife()->keyvals())->container(name));
}

static luabind::object keyvals_list()
{
	return (const_cast<CALifeKeyvalRegistry*>(&ai().get_alife()->keyvals())->list());
}

void keyvals_remove(LPCSTR name)
{
	return (const_cast<CALifeKeyvalRegistry*>(&ai().get_alife()->keyvals())->remove(name));
}

#pragma optimize("s",on)
void CALifeKeyvalContainer::script_register(lua_State *L)
{
	module(L)
	[
		class_<CALifeKeyvalContainer>("CALifeKeyvalContainer")
			.def("get", &CALifeKeyvalContainer::get)
			.def("set", &CALifeKeyvalContainer::set)
			.def("exist", &CALifeKeyvalContainer::exist)
			.def("remove", &CALifeKeyvalContainer::remove)
			.def("clear", &CALifeKeyvalContainer::clear)
			.def("list", &CALifeKeyvalContainer::list)
		
		,def("keyvals", static_cast<CALifeKeyvalContainer* (*)()>(&keyvals))
		,def("keyvals", static_cast<CALifeKeyvalContainer* (*)(LPCSTR)>(&keyvals))
		,def("keyvals_list", &keyvals_list)
		,def("keyvals_remove", &keyvals_remove)
	];
}
