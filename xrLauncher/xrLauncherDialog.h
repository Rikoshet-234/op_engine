#pragma once


// xrLauncherDialog dialog

class xrLauncherDialog : public CDialogEx
{
	DECLARE_DYNAMIC(xrLauncherDialog)

public:
	xrLauncherDialog(CWnd* pParent = nullptr);   // standard constructor
	virtual ~xrLauncherDialog();

// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_DIALOG1 };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
};
