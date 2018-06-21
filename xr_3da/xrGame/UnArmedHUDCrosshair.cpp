// exxZERO Time Stamp AddIn. Document modified at : Thursday, March 07, 2002 14:13:00 , by user : Oles , from computer : OLES
// HUDCursor.cpp: implementation of the CHUDTarget class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "UnArmedHUDCrosshair.h"
#include "hudmanager.h"

#include "../CustomHUD.h"
#include "Entity.h"
#include "level.h"
#include "game_cl_base.h"
#include "color_defs.h"


#include "InventoryOwner.h"
#include "relation_registry.h"
#include "character_info.h"

#include "string_table.h"
#include "entity_alive.h"

#include "inventory_item.h"
#include "inventory.h"

#define C_SIZE		0.025f

#define SHOW_INFO_SPEED		0.5f
#define HIDE_INFO_SPEED		10.f


IC	float	recon_mindist	()		{
	return 2.f;
}
IC	float	recon_maxdist	()		{
	return 50.f;
}
IC	float	recon_minspeed	()		{
	return 0.5f;
}
IC	float	recon_maxspeed	()		{
	return 10.f;
}

void CUnArmedHUDCrosshair::SetCrosshairShader(LPCSTR cursor_tex)
{
	allowRender = false;
	hShader.destroy();
	hGeom.destroy();
	hGeom.create(FVF::F_TL, RCache.Vertex.Buffer(), RCache.QuadIB);
	hShader.create("hud\\cursor", cursor_tex);
	allowRender = true;
}

CUnArmedHUDCrosshair::CUnArmedHUDCrosshair()
{
	fuzzyShowInfo = 0.f;
	allowRender = false;
}

CUnArmedHUDCrosshair::~CUnArmedHUDCrosshair()
{
	hShader.destroy();
	hGeom.destroy();
}


extern ENGINE_API BOOL g_bRendering; 
void CUnArmedHUDCrosshair::Render(collide::rq_result RQ)
{
	if (!allowRender)
		return;
	VERIFY(g_bRendering);

	CObject*	O = Level().CurrentEntity();
	if (!O)	
		return;
	CEntityAlive*	pCurEnt = smart_cast<CEntityAlive*>	(O);
	if (!pCurEnt)
		return;

	Fvector p1 = Device.vCameraPosition;
	Fvector dir = Device.vCameraDirection;

	u32 CursorColor = C_DEFAULT;

	FVF::TL				PT;
	Fvector				p2;
	p2.mad(p1, dir, RQ.range);
	PT.transform(p2, Device.mFullTransform);
	float				di_size = C_SIZE / powf(PT.p.w, .2f);

	CGameFont* F = HUD().Font().pFontGraffiti19Russian;
	F->SetAligment(CGameFont::alCenter);
	F->OutSetI(0.f, 0.05f);

	if (psHUD_Flags.test(HUD_CROSSHAIR_DIST)) {
		F->SetColor(CursorColor);
		F->OutNext("%4.1f", RQ.range);
	}

	if (psHUD_Flags.test(HUD_INFO)) 
	{
		if (RQ.O)
		{
			CEntityAlive*	entytyAlive = smart_cast<CEntityAlive*>	(RQ.O);
			PIItem			l_pI = smart_cast<PIItem>		(RQ.O);
			CInventoryOwner* our_inv_owner = smart_cast<CInventoryOwner*>(pCurEnt);
			if (entytyAlive && entytyAlive->g_Alive() && entytyAlive->cast_base_monster())
			{
				CursorColor = C_ON_ENEMY;
			}
			else if (entytyAlive && entytyAlive->g_Alive() && !entytyAlive->cast_base_monster())
			{
				CInventoryOwner* others_inv_owner = smart_cast<CInventoryOwner*>(entytyAlive);
				if (our_inv_owner && others_inv_owner) {

					switch (RELATION_REGISTRY().GetRelationType(others_inv_owner, our_inv_owner))
					{
					case ALife::eRelationTypeEnemy:
						CursorColor = C_ON_ENEMY; break;
					case ALife::eRelationTypeNeutral:
						CursorColor = C_ON_NEUTRAL; break;
					case ALife::eRelationTypeFriend:
						CursorColor = C_ON_FRIEND; break;
					case ALife::eRelationTypeWorstEnemy:
					case ALife::eRelationTypeLast:
					case ALife::eRelationTypeDummy:
					default:
						CursorColor = C_DEFAULT;
					}

					if (fuzzyShowInfo > 0.5f) {
						CStringTable	strtbl;
						F->SetColor(subst_alpha(CursorColor, u8(iFloor(255.f*(fuzzyShowInfo - 0.5f)*2.f))));
						F->OutNext("%s", *strtbl.translate(others_inv_owner->Name()));
						F->OutNext("%s", *strtbl.translate(others_inv_owner->CharacterInfo().Community().id()));
					}
				}

				fuzzyShowInfo += SHOW_INFO_SPEED*Device.fTimeDelta;
			}
			else if (l_pI && our_inv_owner && RQ.range < 2.0f*our_inv_owner->inventory().GetTakeDist())
			{
				if (fuzzyShowInfo > 0.5f) {
					F->SetColor(subst_alpha(CursorColor, u8(iFloor(255.f*(fuzzyShowInfo - 0.5f)*2.f))));
					F->OutNext("%s", l_pI->Name/*Complex*/());
				}
				fuzzyShowInfo += SHOW_INFO_SPEED*Device.fTimeDelta;
			}

		}
		else {
			fuzzyShowInfo -= HIDE_INFO_SPEED*Device.fTimeDelta;
		}
		clamp(fuzzyShowInfo, 0.f, 1.f);
	}

	// actual rendering
	u32			vOffset;
	FVF::TL*	pv = static_cast<FVF::TL*>(RCache.Vertex.Lock(4, hGeom.stride(), vOffset));

	Fvector2		scr_size;
	scr_size.set(float(::Render->getTarget()->get_width()), float(::Render->getTarget()->get_height()));
	//scr_size.set	(float(Device.dwWidth) ,float(Device.dwHeight));
	float			size_x = scr_size.x	* di_size;
	float			size_y = size_x;// scr_size.y * di_size;

	float			w_2 = scr_size.x / 2.0f;
	float			h_2 = scr_size.y / 2.0f;

	// Convert to screen coords
	float cx = (PT.p.x + 1)*w_2;
	float cy = (PT.p.y + 1)*h_2;

	pv->set(cx - size_x, cy + size_y, CursorColor, 0, 1); ++pv;
	pv->set(cx - size_x, cy - size_y, CursorColor, 0, 0); ++pv;
	pv->set(cx + size_x, cy + size_y, CursorColor, 1, 1); ++pv;
	pv->set(cx + size_x, cy - size_y, CursorColor, 1, 0); ++pv;

	// unlock VB and Render it as triangle LIST
	RCache.Vertex.Unlock(4, hGeom.stride());
	RCache.set_Shader(hShader);
	RCache.set_Geometry(hGeom);
	RCache.Render(D3DPT_TRIANGLELIST, vOffset, 0, 4, 0, 2);
}

