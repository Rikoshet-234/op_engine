#include "pch_script.h"
#include "weaponwalther.h"
#include "ai_space.h"
#include "alife_simulator.h"
#include "alife_object_registry.h"

CWeaponWalther::CWeaponWalther(void) : CWeaponPistol("WALTHER")
{
	m_weight = .5f;
	m_slot = 1;
}

CWeaponWalther::~CWeaponWalther(void)
{
	
}

using namespace luabind;

#pragma optimize("s",on)
void CWeaponWalther::script_register	(lua_State *L)
{
	module(L)
	[
		class_<CWeaponWalther,CGameObject>("CWeaponWalther")
			.def(constructor<>())
	];
}
