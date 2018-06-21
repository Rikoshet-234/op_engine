#include "stdafx.h"
#include "HUDCrosshairManager.h"
#include "Level.h"
#include "HUDManager.h"
#include "actor.h"
#include "Inventory.h"
#include "Weapon.h"

#define LTX_FILE_NAME "hud_crosshairs.ltx"

bool armed_crosshair_custom = false;
int armed_scale_type = CArmedHUDCrosshair::ScaleTypeEnum::distance;
string16 default_crosshair_id = "default"; //:)
shared_str current_unarmed_crosshair;// = default_crosshair_id;


CHUDCrosshairManager::CHUDCrosshairManager()
{
	m_bShowArmedCrosshair = false;
}

shared_str CHUDCrosshairManager::GetCrosshairTex(shared_str crosshair_id)
{
	if (!crosshair_id || crosshairs.find(crosshair_id) == crosshairs.end())
	{
		Msg("~ WARNING crosshair[%s] not found, set to default", crosshair_id.c_str());
		crosshair_id = default_crosshair_id;
	}
	return crosshairs.find(crosshair_id)->second;
}

void CHUDCrosshairManager::Load()
{
	string_path	fname;
	FS.update_path(fname, "$game_config$", LTX_FILE_NAME);
	CInifile* cursorSettings = xr_new<CInifile>(fname, TRUE);
	if (!cursorSettings->section_exist("crosshairs"))
	{
		Msg("! ERROR no required section [crosshairs] in %s", LTX_FILE_NAME);
		FATAL("ENGINE Crush. See log for details.");
	}
	CInifile::Sect S = cursorSettings->r_section("crosshairs");
	std::for_each(S.Data.begin(), S.Data.end(), [&](CInifile::Item line)
	{
		shared_str tex_id = cursorSettings->r_string(line.second, "texture");
		crosshairs.insert(std::make_pair(line.first, tex_id));
	});
	if (crosshairs.find("default")==crosshairs.end())
	{
		Msg("! ERROR no default line 'default' in [crosshairs]");
		FATAL("ENGINE Crush. See log for details.");		
	}
	xr_delete(cursorSettings);
	ReInitUnArmedCrosshair();
	ArmedHUDCrosshair.Load();
	ReInitArmedCrosshair();
}

void CHUDCrosshairManager::SetArmedCrosshairDispersion(float disp)
{
	ArmedHUDCrosshair.SetDispersion(disp);
}

void CHUDCrosshairManager::RenderCrosshair()
{
	collide::rq_result RQ = HUD().GetCurrentRayQuery();
	if (!m_bShowArmedCrosshair)
		UnArmedHUDCrosshair.Render(RQ);
	else
		ArmedHUDCrosshair.Render(RQ);
}

void CHUDCrosshairManager::ReInitUnArmedCrosshair()
{
	UnArmedHUDCrosshair.SetCrosshairShader(GetCrosshairTex(current_unarmed_crosshair).c_str());
}

void CHUDCrosshairManager::ReInitArmedCrosshair()
{
	ArmedHUDCrosshair.ScaleType = static_cast<CArmedHUDCrosshair::ScaleTypeEnum>(armed_scale_type);
	if (armed_crosshair_custom)
	{
		shared_str tex_name;
		if (g_actor)
		{
			CWeapon* pWeapon = smart_cast<CWeapon*>(g_actor->inventory().ActiveItem());
			if (pWeapon && pSettings->line_exist(pWeapon->cNameSect(), "custom_crosshair"))
				tex_name = pSettings->r_string(pWeapon->cNameSect(), "custom_crosshair");
		}
		if (tex_name.size()==0)
			tex_name = GetCrosshairTex(current_unarmed_crosshair).c_str();
		ArmedHUDCrosshair.SetCrosshairType(2, tex_name.c_str());
	}
	else
		ArmedHUDCrosshair.SetCrosshairType(0);
}
