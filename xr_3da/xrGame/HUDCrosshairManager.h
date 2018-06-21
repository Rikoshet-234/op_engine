#pragma once

#include "UnArmedHUDCrosshair.h"
#include "ArmedHUDCrosshair.h"


extern bool armed_crosshair_custom;
extern shared_str current_unarmed_crosshair;
extern int armed_scale_type;

class CHUDCrosshairManager
{
private:
	CUnArmedHUDCrosshair UnArmedHUDCrosshair;
	CArmedHUDCrosshair	ArmedHUDCrosshair;
	xr_map<shared_str, shared_str> crosshairs;
	shared_str GetCrosshairTex(shared_str crosshair_id);
public:
	CHUDCrosshairManager();
	void Load();
	void SetArmedCrosshairDispersion(float disp);
	void RenderCrosshair();
	bool m_bShowArmedCrosshair;
	void ReInitUnArmedCrosshair();
	void ReInitArmedCrosshair();
};

