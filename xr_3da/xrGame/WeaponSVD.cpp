#include "pch_script.h"
#include "weaponsvd.h"

CWeaponSVD::CWeaponSVD(void) : CWeaponCustomPistol("SVD")
{
}

int CWeaponSVD::GetCurrentFireMode()
{
	int fireMode=CWeaponCustomPistol::GetCurrentFireMode();
	if (m_bHasDifferentFireModes)
		fireMode=m_aFireModes[m_iCurFireMode];	
	return fireMode;
}

CWeaponSVD::~CWeaponSVD(void)
{
}

void CWeaponSVD::switch2_Fire	()
{
	//bWorking					= false;
	bWorking					= true;
	m_bFireSingleShot			= true;
	m_bPending					= true;
	m_iShotNum					= 0;
	m_bStopedAfterQueueFired	= false;

}

void CWeaponSVD::OnAnimationEnd(u32 state) 
{
	switch(state) 
	{
	case eFire:	{
		m_bPending = false;
		}break;	// End of reload animation
	}
	inherited::OnAnimationEnd(state);
}

using namespace luabind;

#pragma optimize("s",on)
void CWeaponSVD::script_register	(lua_State *L)
{
	module(L)
	[
		class_<CWeaponSVD,CGameObject>("CWeaponSVD")
			.def(constructor<>())
	];
}
