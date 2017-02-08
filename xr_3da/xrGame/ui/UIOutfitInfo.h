#pragma once

#include "UIWindow.h"
#include "../alife_space.h"
#include "UIListWnd.h"
#include "UIMultiTextStatic.h"
#include "UIXmlInit.h"
#include "xrUIXmlParser.h"
#include "UIWindow.h"

class CUIScrollView;
class CCustomOutfit;
class CUIStatic;
class CUIXml;

class CUIOutfitInfo : public CUIWindow
{
	CCustomOutfit*		m_outfit;
public:
					CUIOutfitInfo			();
	virtual			~CUIOutfitInfo			();

	void Update					(CCustomOutfit* outfit);	
	void UpdateImmuneView();
	void InitFromXml				(CUIXml& xml_doc);
	xr_map<shared_str ,shared_str> iconIDs;
	shared_str xml_path;

protected:
	xr_map<ALife::EHitType,shared_str> immunes;
	//xr_map<int, OPFuncs::restoreParam> restores;
	CUIListWnd*		m_list;
	std::vector<CUIListItemIconed*>	m_lImmuneUnsortedItems;
	std::vector<CUIListItemIconed*>	m_lRestoreUnsortedItems;
	void createImmuneItem(CCustomOutfit* outfit,ALife::EHitType hitType, bool force_add);
	//void createRestoreItem(CCustomOutfit* outfit,int restoreId, bool force_add);
};