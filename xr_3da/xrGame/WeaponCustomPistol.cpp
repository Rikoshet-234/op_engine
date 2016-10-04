#include "stdafx.h"

#include "Entity.h"
#include "WeaponCustomPistol.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
CWeaponCustomPistol::CWeaponCustomPistol(LPCSTR name) : CWeaponMagazined(name,SOUND_TYPE_WEAPON_PISTOL)
{
	m_iCurFireMode=0;
}

CWeaponCustomPistol::~CWeaponCustomPistol()
{
}

int CWeaponCustomPistol::GetCurrentFireMode()
{
	int fireMode=1;
	if (m_bHasDifferentFireModes)
		if (m_aFireModes.size()>0)
			fireMode=m_aFireModes[m_iCurFireMode];	
	return fireMode;
}

void CWeaponCustomPistol::switch2_Fire	()
{
	m_bFireSingleShot			= true;
	bWorking					= false;
	m_iShotNum					= 0;
	m_bStopedAfterQueueFired	= false;
}



void CWeaponCustomPistol::FireEnd() 
{
	if(fTime<=0) 
	{
		m_bPending = false;
		inherited::FireEnd();
	}
}