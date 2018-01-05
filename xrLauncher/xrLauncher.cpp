// xrLauncher.cpp : Defines the exported functions for the DLL application.
//

#include "stdafx.h"

#include "xrLaucher.h"
#include "resource.h"


INT_PTR CALLBACK DialogProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
		case WM_INITDIALOG:
		{
			string512 caption;
			LoadString(g_hInst, IDS_MAIN_TITLE, caption,512);
			SetWindowText(hwndDlg, caption);
			return TRUE;
		}
		default:break;
	}
	return FALSE;
}

LRESULT RunXRLauncher(HWND hwndOwner)
{
	return static_cast<LRESULT>(DialogBox(g_hInst, MAKEINTRESOURCE(IDD_MAIN_XRLAUNCHER_DIALOG), hwndOwner, &DialogProc));;
	//return 
}
