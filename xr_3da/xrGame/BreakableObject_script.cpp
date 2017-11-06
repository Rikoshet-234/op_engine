#include "pch_script.h"
#include "BreakableObject.h"

using namespace luabind;

#pragma optimize("s",on)
void CBreakableObject::script_register(lua_State *L)
{
	module(L)
		[
			class_<CBreakableObject, CGameObject>("CBreakableObject")
			.def(constructor<>())
		];
}
