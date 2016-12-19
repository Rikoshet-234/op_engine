#ifndef UIListItemIconedH
#define UIListItemIconedH

#include "UIItemInfo.h"
#include "script_export_space.h"

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
	//void InitXml(const char *path, CUIXml &uiXml);
	void ClearField(u32 fieldIndex);
	void SetVisibility(u32 fieldIndex,bool visibility);
	bool GetVisibility(u32 fieldIndex);

	xr_vector<CUIStatic*> *GetFields() {return  &fields;}
	DECLARE_SCRIPT_REGISTER_FUNCTION
};

add_to_type_list(CUIListItemIconed)
#undef script_type_list
#define script_type_list save_type_list(CUIListItemIconed)

#endif