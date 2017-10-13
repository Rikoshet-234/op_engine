#include "pch_script.h"
#include "RadioactiveZone.h"

using namespace luabind;

#pragma optimize("s",on)
void CRadioactiveZone::script_register(lua_State *L)
{
	module(L)
		[
			class_<CRadioactiveZone, CGameObject>("CRadioactiveZone")
			.def(constructor<>())
		];
}
