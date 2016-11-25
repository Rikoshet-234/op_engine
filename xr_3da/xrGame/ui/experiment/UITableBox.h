#ifndef UITableBoxH
#define UITableBoxH

#include "UIScrollView.h"
#include "UIStatic.h"
#include "UIFrameWindow.h"
#include "TableColumn.h"

class CUIXml;

class CUITableBox :public CUIScrollView
{
private:
	typedef			CUIScrollView inherited;
	CUIStatic*	rowStaticCache;
	xr_vector<CUITableColumn*> columnsStaticCache;
	xr_map<shared_str ,shared_str> iconIDs;
public:
	CUITableBox				();
	virtual ~CUITableBox();;
	void InitFromXml(CUIXml& xml,LPCSTR elementName);
	void AddRow(xr_vector<shared_str> columnValues);
	void AddIconID(shared_str id,shared_str value);
};
#endif