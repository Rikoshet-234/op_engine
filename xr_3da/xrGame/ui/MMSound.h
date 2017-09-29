#pragma once

class CUIXml;

class CMMSound
{
public:
					CMMSound	();
					~CMMSound	();
	void 			Init		(CUIXml& xml_doc, LPCSTR path);
	void 			whell_Play	();
	void 			whell_Stop	();
	void 			whell_Click	();
	void 			whell_UpdateMoving(float frequency);

	void 			music_Play	(int index=-1);
	void 			music_Stop	();
	void 			music_Update();

	void 			all_Stop	();
	int GetCIP() { return current_index_played;}
	void IncCIP(int step = 1);

	void DecCIP(int step = 1);
protected:

	IC	bool		check_file			(LPCSTR fname);
	ref_sound		m_music_l;
	ref_sound		m_music_r;
	ref_sound		m_whell;
	ref_sound		m_whell_click;
	bool			m_bRandom;
	xr_vector<xr_string>m_play_list;
	int current_index_played;
};
