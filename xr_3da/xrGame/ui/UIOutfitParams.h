#ifndef UIOutfitParams_h
#define UIOutfitParams_h

#include "UIWindow.h"
#include "UIListWnd.h"
#include "UIXmlInit.h"
#include "../alife_space.h"

struct restoreParam;
struct xmlParams;
class CCustomOutfit;
class CInventoryItem;
class CUIXml;

class CUIOutfitParams 
{
public:
	CUIOutfitParams();
	virtual ~CUIOutfitParams();
	void InitFromXml (CUIXml &xml_doc);
	bool Check(CInventoryItem* outfitItem) const;
	bool Check(shared_str section) const;
	void SetInfo(CCustomOutfit* outfitItem,CUIScrollView *parent);
	void SetInfo(const shared_str& outfit_section, CUIScrollView *parent);
	void ClearAll();
	CUIListWnd*		m_list;
private:
	CCustomOutfit* inOutfit;
	xr_map<shared_str ,shared_str> m_mIconIDs;
	xr_map<ALife::EHitType,shared_str> immunes;
	xr_map<int, restoreParam> modificators;
	shared_str currentFileNameXml;
	bool m_bShowModifiers;
	std::vector<CUIListItemIconed*>	m_lImmuneUnsortedItems;
	std::vector<CUIListItemIconed*>	m_lModificatorsUnsortedItems;
	void ClearItems(std::vector<CUIListItemIconed*> &baseList) const;	
	void createImmuneItem(shared_str outfit_section,std::pair<ALife::EHitType,shared_str> immunePair, bool force_add);
	void createModifItem(shared_str outfit_section,std::pair<int, restoreParam> modifPair, bool force_add);
};

#endif