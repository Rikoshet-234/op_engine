#include "stdafx.h"
#pragma hdrstop

#include "SoundRender_CoreA.h"
#include "SoundRender_CoreD.h"

void CSound_manager_interface::_create		(u64 window, LPCSTR localization)
{
	if (strstr( Core.Params,"-dsound"))
	{
		SoundRenderD	= xr_new<CSoundRender_CoreD>();
		SoundRender		= SoundRenderD;
		Sound			= SoundRender;
	}else{
		SoundRenderA	= xr_new<CSoundRender_CoreA>();
		SoundRender		= SoundRenderA;
		Sound			= SoundRender;
	}
	if (strstr			( Core.Params,"-nosound")){
		SoundRender->bPresent = FALSE;
		return;
	}
	Sound->_initialize	(window);
	Sound->_localization(localization);
}

void CSound_manager_interface::_destroy	()
{
	Sound->_clear		();
	xr_delete			(SoundRender);
	Sound				= 0;
}

void CSound_manager_interface::_check_ogg(LPCSTR oggFile)
{
	SoundRender->_check_ogg(oggFile);
}

