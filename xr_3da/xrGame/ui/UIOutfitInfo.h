#pragma once

#include "UIWindow.h"
#include "../alife_space.h"
#include "UIListWnd.h"
#include "UIMultiTextStatic.h"
#include "UIXmlInit.h"
#include "xrUIXmlParser.h"
#include "UIWindow.h"

struct restoreParam;
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
	void InitFromXml				(CUIXml &xml_doc);
	xr_map<shared_str ,shared_str> iconIDs;
	CUIListWnd*		m_list;
	void ClearAll();
protected:
	bool m_bShowModifiers;
	xr_map<ALife::EHitType,shared_str> immunes;
	xr_map<int, restoreParam> modificators;
	std::vector<float> artefactRestores;
	void ClearItems(std::vector<CUIListItemIconed*> &baseList);	
	std::vector<CUIListItemIconed*>	m_lImmuneUnsortedItems;
	std::vector<CUIListItemIconed*>	m_lModificatorsUnsortedItems;
	void createImmuneItem(CCustomOutfit* outfit,std::pair<ALife::EHitType,shared_str> immunePair, bool force_add);
	void createModifItem(CCustomOutfit* outfit,std::pair<int, restoreParam> modifPair, bool force_add);
};