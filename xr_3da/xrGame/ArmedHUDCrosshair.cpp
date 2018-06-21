// HUDCrosshair.cpp:  крестик прицела, отображающий текущую дисперсию
// 
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"

#include "ArmedHUDCrosshair.h"
#include "../CustomHUD.h"
#include "Level.h"
#include "Entity_Alive.h"
#include "color_defs.h"
#include "relation_registry.h"
#include "InventoryOwner.h"

CArmedHUDCrosshair::CArmedHUDCrosshair()
{
	radius = 0;
	ch_type = 1;
	allowRender = true;
	ScaleType = ScaleTypeEnum::distance;
}


CArmedHUDCrosshair::~CArmedHUDCrosshair	()
{
	hGeomLine.destroy			();
	hShader.destroy				();
}

void CArmedHUDCrosshair::Load		()
{
	cross_length_perc = pSettings->r_float(HUD_CURSOR_SECTION, "cross_length");
	min_radius_perc = pSettings->r_float(HUD_CURSOR_SECTION, "min_radius");
	max_radius_perc = pSettings->r_float(HUD_CURSOR_SECTION, "max_radius");
	cross_color = pSettings->r_fcolor(HUD_CURSOR_SECTION, "cross_color").get();
	radius_speed_perc = pSettings->r_float(HUD_CURSOR_SECTION, "radius_lerp_speed");
}

void CArmedHUDCrosshair::SetCrosshairType(int crosshair_type, LPCSTR tex_name)
{
	ch_type = crosshair_type;
	allowRender = false;
	hShader.destroy();
	hGeomLine.destroy();
	switch (ch_type)
	{
	case 0:
		{
			hGeomLine.create(FVF::F_TL0uv, RCache.Vertex.Buffer(), nullptr);
			hShader.create("hud\\cursor");
		}
		break;
	case 2:
		{
			hGeomLine.create(FVF::F_TL, RCache.Vertex.Buffer(), RCache.QuadIB);
			hShader.create("hud\\cursor", tex_name);
		}
		break;
	}
	allowRender = true;
}

//выставляет radius от min_radius до max_radius
void CArmedHUDCrosshair::SetDispersion	(float disp)
{ 
	Fvector4 r;
	Fvector R			= { VIEWPORT_NEAR*_sin(disp), 0.f, VIEWPORT_NEAR };
	Device.mProject.transform	(r,R);

	Fvector2		scr_size;
	scr_size.set	(float(::Render->getTarget()->get_width()), float(::Render->getTarget()->get_height()));
	float radius_pixels		= _abs(r.x)*scr_size.x/2.0f;
	//	clamp(radius_pixels, min_radius, max_radius);
	target_radius		= radius_pixels; 
}

void CArmedHUDCrosshair::DrawSoCCrosshair(Fvector2 &center, Fvector2 scr_size)
{
	// draw back
	u32			dwOffset, dwCount;
	dwCount = 10;
	FVF::TL0uv* pv_start = (FVF::TL0uv*)RCache.Vertex.Lock(dwCount, hGeomLine->vb_stride, dwOffset);
	FVF::TL0uv* pv = pv_start;


	float cross_length = cross_length_perc*scr_size.x;
	float min_radius = min_radius_perc*scr_size.x;
	float max_radius = max_radius_perc*scr_size.x;

	clamp(target_radius, min_radius, max_radius);

	float x_min = min_radius + radius;
	float x_max = x_min + cross_length;

	float y_min = x_min;
	float y_max = x_max;

	// 0
	pv->set(center.x + 1, center.y + y_min, cross_color); pv++;
	pv->set(center.x + 1, center.y + y_max, cross_color); pv++;
	// 1
	pv->set(center.x + 1, center.y - y_min, cross_color); pv++;
	pv->set(center.x + 1, center.y - y_max, cross_color); pv++;
	// 2
	pv->set(center.x + x_min + 1, center.y, cross_color); pv++;
	pv->set(center.x + x_max + 1, center.y, cross_color); pv++;
	// 3
	pv->set(center.x - x_min, center.y, cross_color); pv++;
	pv->set(center.x - x_max, center.y, cross_color); pv++;
	// 4
	pv->set(center.x, center.y, cross_color); pv++;
	pv->set(center.x + 1, center.y, cross_color); pv++;
	//*/
	// render	
	RCache.Vertex.Unlock(dwCount, hGeomLine->vb_stride);

	RCache.set_Shader(hShader);
	RCache.set_Geometry(hGeomLine);
	RCache.Render(D3DPT_LINELIST, dwOffset, dwCount / 2);
}

void CArmedHUDCrosshair::Draw1Crosshair(Fvector2& center, Fvector2 scr_size)
{
	u32			dwOffset, dwCount;
	dwCount = 6;
	FVF::TL0uv* pv_start = (FVF::TL0uv*)RCache.Vertex.Lock(dwCount, hGeomLine->vb_stride, dwOffset);
	FVF::TL0uv* pv = pv_start;
	pv->set(center.x - scr_size.x, center.y + scr_size.y,0.0f, 0.0f, cross_color);
	pv->set(center.x - scr_size.x, center.y - scr_size.y, 0.0f, 0.0f, cross_color);
	pv->set(center.x + scr_size.x, center.y + scr_size.y, 0.0f, 0.0f, cross_color);
	//	Tri 2
	pv->set(center.x + scr_size.x, center.y + scr_size.y, 0.0f, 0.0f, cross_color);
	pv->set(center.x - scr_size.x, center.y - scr_size.y, 0.0f, 0.0f, cross_color);
	pv->set(center.x + scr_size.x, center.y - scr_size.y, 0.0f, 0.0f, cross_color);

	RCache.Vertex.Unlock(dwCount, hGeomLine->vb_stride);

	RCache.set_Shader(hShader);
	RCache.set_Geometry(hGeomLine);
	RCache.Render(D3DPT_TRIANGLELIST, dwOffset, dwCount/3);

}

void CArmedHUDCrosshair::DrawCustomTex(Fvector2& center, Fvector2 scr_size, float range)
{
	u32			vOffset;
	FVF::TL*	pv = static_cast<FVF::TL*>(RCache.Vertex.Lock(4, hGeomLine.stride(), vOffset));

	float delta = 0.f;
	switch (ScaleType)
	{
		case (ScaleTypeEnum::dispersion):
		{
			float cross_length = cross_length_perc*scr_size.x;
			float min_radius = min_radius_perc*scr_size.x;
			float max_radius = max_radius_perc*scr_size.x;
			clamp(target_radius, min_radius, max_radius);
			float x_min = min_radius + radius;
			float x_max = x_min + cross_length;
			delta = (x_min + x_max) / 2.0f;
		}
		break;
		case (ScaleTypeEnum::distance):
		{
			Fvector p1 = Device.vCameraPosition;
			Fvector dir = Device.vCameraDirection;
			FVF::TL				PT;
			Fvector				p2;
			p2.mad(p1, dir, range);
			PT.transform(p2, Device.mFullTransform);
			float				di_size = 0.025f / powf(PT.p.w, .2f);
			delta = scr_size.x	* di_size;
		}
		break;
	}
	
	pv->set(center.x - delta, center.y + delta, cross_color, 0, 1); ++pv;
	pv->set(center.x - delta, center.y - delta, cross_color, 0, 0); ++pv;
	pv->set(center.x + delta, center.y + delta, cross_color, 1, 1); ++pv;
	pv->set(center.x + delta, center.y - delta, cross_color, 1, 0); ++pv;
	RCache.Vertex.Unlock(4, hGeomLine.stride());
	RCache.set_Shader(hShader);
	RCache.set_Geometry(hGeomLine);
	RCache.Render(D3DPT_TRIANGLELIST, vOffset, 0, 4, 0, 2);
}

extern ENGINE_API BOOL g_bRendering; 
void CArmedHUDCrosshair::Render(collide::rq_result RQ)
{
	VERIFY			(g_bRendering);
	if (!allowRender)
		return;
	cross_color = C_DEFAULT;
	if (psHUD_Flags.test(HUD_INFO) && RQ.O)
	{
		CEntityAlive*	entytyAlive = smart_cast<CEntityAlive*>	(RQ.O);
		CEntityAlive*	pCurEnt = smart_cast<CEntityAlive*>	(Level().CurrentEntity());
		CInventoryOwner* our_inv_owner = smart_cast<CInventoryOwner*>(pCurEnt);
		if (entytyAlive && entytyAlive->g_Alive() && entytyAlive->cast_base_monster())
		{
			cross_color = C_ON_ENEMY;
		}
		else if (entytyAlive && entytyAlive->g_Alive() && !entytyAlive->cast_base_monster())
		{
			CInventoryOwner* others_inv_owner = smart_cast<CInventoryOwner*>(entytyAlive);
			if (our_inv_owner && others_inv_owner) {

				switch (RELATION_REGISTRY().GetRelationType(others_inv_owner, our_inv_owner))
				{
				case ALife::eRelationTypeEnemy:
					cross_color = C_ON_ENEMY; break;
				case ALife::eRelationTypeNeutral:
					cross_color = C_ON_NEUTRAL; break;
				case ALife::eRelationTypeFriend:
					cross_color = C_ON_FRIEND; break;
				case ALife::eRelationTypeWorstEnemy: break;
				case ALife::eRelationTypeLast: break;
				case ALife::eRelationTypeDummy: break;
				default:;
				}
			}
		}
	}
	Fvector2		center;
	Fvector2		scr_size;
	scr_size.set	(float(::Render->getTarget()->get_width()), float(::Render->getTarget()->get_height()));
	center.set		(scr_size.x/2.0f, scr_size.y/2.0f);

	switch (ch_type)
	{
	case 0:
		DrawSoCCrosshair(center, scr_size);
		break;
		//case 1:
		//	Draw1Crosshair(center, scr_size);
		//	break;
	case 2:
		DrawCustomTex(center, scr_size,RQ.range);
		break;
	}

	if(!fsimilar(target_radius,radius))
	{
		float sp				= radius_speed_perc * scr_size.x ;
		float radius_change		= sp*Device.fTimeDelta;
		clamp					(radius_change, 0.0f, sp*0.033f); // clamp to 30 fps
		clamp					(radius_change, 0.0f, _abs(target_radius-radius));

		if(target_radius < radius)
			radius -= radius_change;
		else
			radius += radius_change;
	};
}