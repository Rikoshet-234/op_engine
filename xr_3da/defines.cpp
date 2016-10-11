#include "stdafx.h"

#ifdef DEBUG
	ECORE_API BOOL bDebug	= FALSE;
#endif

// Video
//. u32			psCurrentMode		= 1024;
u32			psCurrentVidMode[2] = {1024,768};
u32			psCurrentBPP		= 32;
// release version always has "mt_*" enabled
Flags32		psDeviceFlags		= {rsFullscreen|rsDetails|mtPhysics|mtSound|mtNetwork|rsDrawStatic|rsDrawDynamic};

// textures 
int			psTextureLOD		= 0;

Flags32 g_uCommonFlags;
int			c_r		=0;
int			c_g		=0;
int			c_b		=0;
int			c_a		=0;
int			c_c		=0;
