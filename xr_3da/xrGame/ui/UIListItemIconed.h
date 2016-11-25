#ifndef UIListItemIconedH
#define UIListItemIconedH

#include "UIItemInfo.h"

class CUIListItemIconed : public CUIListItem
{
private:
	xr_vector<CUIStatic*> fields;
	CUIStatic* GetFieldStatic(u32 fieldIndex);
public:
	virtual ~CUIListItemIconed();
	void SetFieldText(u32 fieldIndex, LPCSTR value);
	LPCSTR GetFieldText(u32 fieldIndex);
	void SetFieldIcon(u32 fieldIndex, LPCSTR textureName);
	void InitXml(const char *path, CUIXml &uiXml);
	void ClearField(u32 fieldIndex);
	void SetVisibility(u32 fieldIndex,bool visibility);
};


#endif