#pragma once

#include "UIWindow.h"
#include "../alife_space.h"
#include "UIListWnd.h"
#include "UIMultiTextStatic.h"
#include "UIXmlInit.h"
#include "xrUIXmlParser.h"

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
	void NewSetItem(ALife::EHitType hitType, bool force_add);
	CUIListWnd*		m_list;

};