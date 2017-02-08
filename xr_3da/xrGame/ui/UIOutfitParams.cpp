#include "stdafx.h"
#include "UIOutfitParams.h"
#include "UIXmlInit.h"
#include "UIListWnd.h"
#include "../inventory_item.h"
#include "../CustomOutfit.h"

std::pair<int,shared_str> createPair(ALife::EHitType hitType)
{
	string128 str_desc;
	sprintf_s(str_desc,"ui_inv_outfit_%s_protection",ALife::g_cafHitType2String(hitType));
	return mk_pair(hitType,str_desc);
}

CUIOutfitParams::CUIOutfitParams()
{
	immunes.insert(createPair(ALife::eHitTypeBurn));
	immunes.insert(createPair(ALife::eHitTypeShock));
	immunes.insert(createPair(ALife::eHitTypeStrike));
	immunes.insert(createPair(ALife::eHitTypeWound));
	immunes.insert(createPair(ALife::eHitTypeRadiation));
	immunes.insert(createPair(ALife::eHitTypeTelepatic));
	immunes.insert(createPair(ALife::eHitTypeChemicalBurn));
	immunes.insert(createPair(ALife::eHitTypeExplosion));
	immunes.insert(createPair(ALife::eHitTypeFireWound));
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
