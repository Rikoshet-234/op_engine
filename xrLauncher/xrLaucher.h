#pragma once
#ifndef xrLauncher_h
#define xrLauncher_h
#include <windows.h>

extern "C"
{
	LRESULT __declspec(dllexport) RunXRLauncher(HWND hwndOwner);
};

#endif
