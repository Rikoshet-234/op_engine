#include "stdafx.h"

#ifdef DEBUG
	ECORE_API BOOL bDebug	= FALSE;
#endif

// Video
//. u32			psCurrentMode		= 1024;
	u32	psCurrentVidMode[2] = { 1024,768 };
	u32	psCurrentBPP = 32;
	u32 psCurrentFontProfileIndex = 0;
	u32 psCurrentLanguageIndex = (u32)-1;
// release version always has "mt_*" enabled
	Flags32		psDeviceFlags = { rsFullscreen | rsDetails | mtPhysics | mtSound | mtNetwork | rsDrawStatic | rsDrawDynamic };
	Flags32		g_uCommonFlags = { flAiUseTorchDynamicLights | uiShowConditions | uiShowExtDesc | uiShowFocused | gpDeferredReload | gpFixedReload | uiShowTradeSB };
// textures 
	int			psTextureLOD = 0;

	float lpLoadScreenTextOffsetX = 0;
	float lpLoadScreenTextOffsetY = 0;
	bool lpLoadScreenEnableProgressBar = true;
