#include "stdafx.h"
#include "UIOutfitParams.h"
#include "UIXmlInit.h"
#include "UIListWnd.h"
#include "../inventory_item.h"
#include "../CustomOutfit.h"
#include "../OPFuncs/utils.h"


CUIOutfitParams::CUIOutfitParams()
{
	immunes = OPFuncs::CreateImmunesStringMap();
	modificators = OPFuncs::CreateRestoresStringMap();
}

CUIOutfitParams::~CUIOutfitParams()
{
}

void CUIOutfitParams::InitFromXml(CUIXml &xml_doc) 
{
	LPCSTR _base				= "outfit_params";
	string256					_buff;
	if (!xml_doc.NavigateToNode(_base, 0))	return;
	CUIXmlInit::InitListWnd(xml_doc,_base,0,this);
	strconcat(sizeof(_buff),_buff, _base, ":immunities_list:icons");
	CUIXmlInit::GetStringTable(xml_doc,_buff,0,m_mIconIDs);
}

bool CUIOutfitParams::Check(CInventoryItem* outfitItem) const
{
	return smart_cast<CCustomOutfit*>(outfitItem)!=nullptr;
}

void CUIOutfitParams::SetInfo(CInventoryItem* outfitItem) 
{
	RemoveAll();
	std::for_each(immunes.begin(),immunes.end(),[](std::pair<int,shared_str> immunePair)
	{
		
	});
	/*NewSetItem(ALife::eHitTypeBurn,			false);
	NewSetItem(ALife::eHitTypeShock,		false);
	NewSetItem(ALife::eHitTypeStrike,		false);
	NewSetItem(ALife::eHitTypeWound,		false);
	NewSetItem(ALife::eHitTypeRadiation,	false);
	NewSetItem(ALife::eHitTypeTelepatic,	false);
	NewSetItem(ALife::eHitTypeChemicalBurn,	false);
	NewSetItem(ALife::eHitTypeExplosion,	false);
	NewSetItem(ALife::eHitTypeFireWound,	false)*/
}
