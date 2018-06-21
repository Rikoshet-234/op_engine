// HUDCrosshair.h:  крестик прицела, отображающий текущую дисперсию
// 
//////////////////////////////////////////////////////////////////////

#pragma once

#define HUD_CURSOR_SECTION "hud_cursor"


class CArmedHUDCrosshair	
{
private:
	float			cross_length_perc;
	float			min_radius_perc;
	float			max_radius_perc;

	//текущий радиус прицела
	float			radius;
	float			target_radius;
	float			radius_speed_perc; 

	ref_geom 		hGeomLine;
	ref_shader		hShader;
	int ch_type;
	bool allowRender;
	void DrawSoCCrosshair(Fvector2 &center, Fvector2 scr_size);
	void Draw1Crosshair(Fvector2 &center, Fvector2 scr_size);
	void DrawCustomTex(Fvector2 &center, Fvector2 scr_size,float range);
public:
	u32				cross_color;
	enum ScaleTypeEnum
	{
		dispersion,
		distance
	} ScaleType;

					CArmedHUDCrosshair	();
					~CArmedHUDCrosshair	();

			void	Render		(collide::rq_result RQ);
			void	SetDispersion	(float disp);

			void	Load			();
			void SetCrosshairType(int crosshair_type,LPCSTR tex_name=nullptr);

};