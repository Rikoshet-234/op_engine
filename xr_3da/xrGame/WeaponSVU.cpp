#include "pch_script.h"
#include "weaponsvu.h"

void CWeaponSVU::switch2_Fire()
{
	bWorking					= true;
	m_bFireSingleShot			= true;
	m_bPending					= true;
	m_iShotNum					= 0;
	m_bStopedAfterQueueFired	= false;
}

CWeaponSVU::CWeaponSVU(void) : CWeaponCustomPistol("SVU")
{
}

CWeaponSVU::~CWeaponSVU(void)
{
}

using namespace luabind;

#pragma optimize("s",on)
void CWeaponSVU::script_register	(lua_State *L)
{
	module(L)
	[
		class_<CWeaponSVU,CGameObject>("CWeaponSVU")
			.def(constructor<>())
	];
}
