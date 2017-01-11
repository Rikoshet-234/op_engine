#include "stdafx.h"
#include "WeaponBinoculars.h"

#include "xr_level_controller.h"

#include "level.h"
#include "ui\UIFrameWindow.h"
#include "WeaponBinocularsVision.h"
#include "object_broker.h"
#include "hudmanager.h"
CWeaponBinoculars::CWeaponBinoculars() : CWeaponCustomPistol("BINOCULARS")
{
	m_binoc_vision	= nullptr;
	m_bVision		= false;
	m_fScopeZoomStepCount=3;
}

CWeaponBinoculars::~CWeaponBinoculars()
{
	xr_delete				(m_binoc_vision);
}

void CWeaponBinoculars::Load	(LPCSTR section)
{
	inherited::Load(section);

	m_bVision = !!pSettings->r_bool(section,"vision_present");
}


bool CWeaponBinoculars::Action(s32 cmd, u32 flags) 
{
	if (cmd==kWPN_FIRE && iMagazineSize==0) //only for binoculars, must set in config file ammo_mag_size=0
		return inherited::Action(kWPN_ZOOM, flags);
	return inherited::Action(cmd, flags); //for other weapons, based on WP_BINOC
}

void CWeaponBinoculars::OnZoomIn		()
{
	if(H_Parent() && !IsZoomed())
		if(m_bVision && !m_binoc_vision) 
			m_binoc_vision	= xr_new<CBinocularsVision>(this);
	inherited::OnZoomIn();
}

void CWeaponBinoculars::OnZoomOut		()
{
	if(H_Parent() && IsZoomed() && !IsRotatingToZoom())
		if(m_binoc_vision)
			xr_delete		(m_binoc_vision);
	inherited::OnZoomOut();
}

BOOL	CWeaponBinoculars::net_Spawn			(CSE_Abstract* DC)
{
	inherited::net_Spawn(DC);
	return TRUE;
}

void	CWeaponBinoculars::net_Destroy()
{
	inherited::net_Destroy();
	xr_delete(m_binoc_vision);
}

void	CWeaponBinoculars::UpdateCL()
{
	inherited::UpdateCL();
	//manage visible entities here...
	if(H_Parent() && IsZoomed() && !IsRotatingToZoom() && m_binoc_vision)
		m_binoc_vision->Update();
}

void CWeaponBinoculars::OnDrawUI()
{
	if(H_Parent() && IsZoomed() && !IsRotatingToZoom() && m_binoc_vision)
		m_binoc_vision->Draw();
	inherited::OnDrawUI	();
}

void CWeaponBinoculars::save(NET_Packet &output_packet)
{
	inherited::save(output_packet);
}

void CWeaponBinoculars::load(IReader &input_packet)
{
	inherited::load(input_packet);
}

void CWeaponBinoculars::GetBriefInfo(xr_string& str_name, xr_string& icon_sect_name, xr_string& str_count)
{
	/*str_name		= NameShort();
	str_count		= "";
	icon_sect_name	= *cNameSect();*/
	inherited::GetBriefInfo(str_name,icon_sect_name,str_count);
}

void CWeaponBinoculars::net_Relcase	(CObject *object)
{
	if (!m_binoc_vision)
		return;

	m_binoc_vision->remove_links	(object);
}

int CWeaponBinoculars::GetCurrentFireMode()
{
	 return CWeaponCustomPistol::GetCurrentFireMode();
}

void CWeaponBinoculars::FireEnd()
{
	CWeaponMagazined::FireEnd();
}

void CWeaponBinoculars::switch2_Fire()
{
	CWeaponMagazined::switch2_Fire();
}


