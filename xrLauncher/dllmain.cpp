#include "stdafx.h"

#include <windows.h>

HINSTANCE g_hInst = nullptr;

BOOL APIENTRY DllMain( HMODULE hInst, DWORD  ul_reason_for_call, LPVOID )
{
	if (ul_reason_for_call == DLL_PROCESS_ATTACH)
		g_hInst = hInst;
	return TRUE;
}



