#pragma once

class CUnArmedHUDCrosshair 
{
private:
	ref_shader		hShader;
	ref_geom		hGeom;
	float			fuzzyShowInfo;
	bool allowRender;
public:
					CUnArmedHUDCrosshair		();
					~CUnArmedHUDCrosshair();
			void	Render			(collide::rq_result RQ);
			void SetCrosshairShader(LPCSTR cursor_tex);
};
