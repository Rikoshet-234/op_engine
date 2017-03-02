#ifndef UIOutfitParams_h
#define UIOutfitParams_h

#include "UIWindow.h"
#include "UIListWnd.h"
#include "../alife_space.h"

class CInventoryItem;
class CUIXml;

namespace OPFuncs {
	struct restoreParam;
}

class CUIOutfitParams :public CUIListWnd
{
public:
	CUIOutfitParams();
	virtual ~CUIOutfitParams();
	void InitFromXml (CUIXml& xml_doc);
	bool Check(CInventoryItem* outfitItem) const;
	void SetInfo(CInventoryItem* outfitItem) ;
private:
	xr_map<shared_str ,shared_str> m_mIconIDs;
	xr_map<ALife::EHitType,shared_str> immunes;
	xr_map<int, OPFuncs::restoreParam> modificators;
};

#endif