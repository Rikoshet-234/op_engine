#pragma once
#include "UIWindow.h"
#include "IconedItemsHelper.h"

class CUIXml;
class CUIStatic;

class CUIArtefactParams :public CUIWindow
{
public:
								CUIArtefactParams		();
	virtual						~CUIArtefactParams		();
	void 						InitFromXml				(CUIXml& xml_doc);
	bool 						Check					(const shared_str& af_section) const;
	bool						Check(CInventoryItem* item) const;
	void 						SetInfo					(const shared_str& af_section,CUIScrollView *parent);
	void 						SetInfo					(CInventoryItem* item,CUIScrollView *parent);
	void ClearAll();
	CUIListWnd*		m_list;
private:
	xr_map<shared_str ,shared_str> m_mIconIDs;
	xr_map<ALife::EHitType,shared_str> immunes;
	xr_map<int, restoreParam> modificators;
	shared_str currentFileNameXml;
	bool m_bShowModifiers;
	std::vector<CUIListItemIconed*>	m_lImmuneUnsortedItems;
	std::vector<CUIListItemIconed*>	m_lModificatorsUnsortedItems;
	void ClearItems(std::vector<CUIListItemIconed*> &baseList);	
	void createImmuneItem(shared_str af_section,std::pair<ALife::EHitType,shared_str> immunePair, bool force_add);
	void createModifItem(shared_str af_section,std::pair<int, restoreParam> modifPair, bool force_add);
protected:
	
};