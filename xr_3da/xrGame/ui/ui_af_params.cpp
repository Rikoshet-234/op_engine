#include "stdafx.h"
#include "ui_af_params.h"
#include "UIStatic.h"
#include "../object_broker.h"
#include "UIXmlInit.h"
#include "../Artifact.h"
#include "../OPFuncs/utils.h"
#include "UIScrollView.h"

CUIArtefactParams::CUIArtefactParams()
{
	immunes = CreateImmunesStringMap();
	modificators = CreateModificatorsStringMap();
	m_list=nullptr;
	m_bShowModifiers=false;
}

CUIArtefactParams::~CUIArtefactParams()
{
	ClearAll();
	xr_delete(m_list);
}

#define PARAMS_PATH "af_params:immunities_list"

void CUIArtefactParams::InitFromXml(CUIXml& xml_doc)
{
	string256					_buff;
	m_list=xr_new<CUIListWnd>();
	m_list->SetAutoDelete(false);
	CUIXmlInit::InitListWnd(xml_doc,PARAMS_PATH,0,m_list);
	m_list->EnableScrollBar(false);
	m_list->EnableAlwaysShowScroll(false);
	m_list->SetIgnoreScrolling(true);
	m_bShowModifiers=xml_doc.ReadAttribInt(PARAMS_PATH,0,"show_modifiers",0)==1?true:false;
	strconcat(sizeof(_buff),_buff, PARAMS_PATH, ":icons");
	CUIXmlInit::GetStringTable(xml_doc,_buff,0,m_mIconIDs);
	currentFileNameXml= OPFuncs::getFileNameFromPath(xml_doc.m_xml_file_name).c_str();
}

bool CUIArtefactParams::Check(const shared_str& af_section) const
{
	return !!pSettings->line_exist(af_section, "af_actor_properties");
}

bool CUIArtefactParams::Check(CInventoryItem* item) const
{
	return smart_cast<CArtefact*>(item)!=nullptr;
}

void CUIArtefactParams::SetInfo(const shared_str& af_section,CUIScrollView *parent)
{
	m_list->RemoveAll();
#pragma region update immune lines
	std::for_each(immunes.begin(),immunes.end(),[&](std::pair<ALife::EHitType,shared_str> immunePair)
	{
		createImmuneItem(af_section,immunePair,false);
	});
	if (m_lImmuneUnsortedItems.size()>0)
	{
		std::sort(m_lImmuneUnsortedItems.begin(),m_lImmuneUnsortedItems.end(),[](CUIListItem* i1, CUIListItem* i2)
		{
			CUIListItemIconed *iconedItem1=smart_cast<CUIListItemIconed*>(i1);
			CUIListItemIconed *iconedItem2=smart_cast<CUIListItemIconed*>(i2);
			if (!iconedItem1 || !iconedItem2)
				return false;
			return		lstrcmpi(iconedItem1->GetFieldText(1),iconedItem2->GetFieldText(1))<0;
		});
		std::for_each(m_lImmuneUnsortedItems.begin(),m_lImmuneUnsortedItems.end(),[&](CUIListItemIconed* item)
		{
			m_list->AddItem<CUIListItemIconed>(item);
		});
	}
#pragma endregion
	#pragma region update modifiers lines
	if (m_bShowModifiers)
	{
		float addWeight=READ_IF_EXISTS(pSettings,r_float,af_section,"additional_weight",0);
		CUIListItemIconed* weightItem= findIconedItem(m_lModificatorsUnsortedItems,"additional_weight",!!fsimilar(addWeight, 0.0f) ,xmlParams(currentFileNameXml,PARAMS_PATH));
		if (weightItem)
			setIconedItem(m_mIconIDs,weightItem,"additional_weight","ui_inv_outfit_additional_inventory_weight",addWeight,1,0,-1);
		std::for_each(modificators.begin(),modificators.end(),[&](std::pair<int, restoreParam> modifPair)
		{
			createModifItem(af_section,modifPair,false);
		});
		if (m_lModificatorsUnsortedItems.size()>0)
		{
			if (m_lImmuneUnsortedItems.size()>0)
				addSeparator(m_list);
			std::sort(m_lModificatorsUnsortedItems.begin(),m_lModificatorsUnsortedItems.end(),[](CUIListItem* i1, CUIListItem* i2)
			{
				CUIListItemIconed *iconedItem1=smart_cast<CUIListItemIconed*>(i1);
				CUIListItemIconed *iconedItem2=smart_cast<CUIListItemIconed*>(i2);
				if (!iconedItem1 || !iconedItem2)
					return false;
				return		lstrcmpi(iconedItem1->GetFieldText(1),iconedItem2->GetFieldText(1))<0;
			});
			std::for_each(m_lModificatorsUnsortedItems.begin(),m_lModificatorsUnsortedItems.end(),[&](CUIListItemIconed* item)
			{
				m_list->AddItem<CUIListItemIconed>(item);
			});
		}
	}
#pragma endregion
	m_list->SetHeight(m_list->GetItemsCount()*m_list->GetItemHeight()+5);
	m_list->GetInternalScrollbar()->SetPageSize(m_list->GetItemsCount());
	m_list->ScrollToBegin();
	parent->AddWindow(m_list,false);
}

void CUIArtefactParams::SetInfo(CInventoryItem* item,CUIScrollView *parent)
{
	SetInfo(item->object().cNameSect(),parent);
}

void CUIArtefactParams::ClearAll()
{
	ClearItems(m_lImmuneUnsortedItems);
	ClearItems(m_lModificatorsUnsortedItems);
}

void CUIArtefactParams::ClearItems(std::vector<CUIListItemIconed*>& baseList)
{
	while(!baseList.empty())
	{
		auto item=baseList.front();
		baseList.erase(std::remove(baseList.begin(),baseList.end(),item),baseList.end());
		item->DetachAll();
		xr_delete(item);
	}
}

void CUIArtefactParams::createImmuneItem(shared_str af_section, std::pair<ALife::EHitType, shared_str> immunePair, bool force_add)
{
	LPCSTR hitName= ALife::g_cafHitType2String(immunePair.first);
	string256 buff;
	sprintf_s(buff,"%s_immunity",hitName);
	shared_str _sect	= pSettings->r_string(af_section, "hit_absorbation_sect");
	float art_val				= pSettings->r_float(_sect, buff);
	bool emptyParam=fsimilar(art_val, 1.0f) && !force_add;
	art_val= (1.0f - art_val);
	CUIListItemIconed* item= findIconedItem(m_lImmuneUnsortedItems,hitName,emptyParam,xmlParams(currentFileNameXml,PARAMS_PATH));
	if (!item)
		return;
	setIconedItem(m_mIconIDs,item,hitName,immunePair.second,art_val,0,0,-1);
}

void CUIArtefactParams::createModifItem(shared_str af_section, std::pair<int, restoreParam> modifPair, bool force_add)
{
	float artValue=READ_IF_EXISTS(pSettings,r_float,af_section, modifPair.second.paramName.c_str(),0);
	float actorValue= READ_IF_EXISTS(pSettings,r_float,"actor_condition", modifPair.second.actorParamName.c_str(),1);
	switch (modifPair.first)
	{
		case JUMP_SPEED_DELTA_ID:
			break;
		case BLEEDING_RESTORE_ID:
			{
				artValue = (artValue/actorValue)*100.0f*-1.0f;
			}
			break;
		case SATIETY_RESTORE_ID:
			{
				artValue = (artValue/actorValue)*100.0f;				
			}
			break;
		case RADIATION_RESTORE_ID:
			{
					artValue = (artValue/actorValue);
			}
			break;
		case HEALTH_RESTORE_ID:
			{
					artValue = (artValue/actorValue)*100.0f;
			}
			break;
		case POWER_RESTORE_ID:
			{
					artValue = (artValue/actorValue);
			}
			break;
		case POWER_LOSS_ID:
			{
					artValue = (artValue/actorValue)*100.0f;
			}
			break;
		default:
			return;
	}
	bool emptyParam=fsimilar(artValue, 0.0f) && !force_add;
	CUIListItemIconed* item= findIconedItem(m_lModificatorsUnsortedItems,modifPair.second.paramName.c_str(),emptyParam,xmlParams(currentFileNameXml,PARAMS_PATH));
	if (!item)
		return;
	setIconedItem(m_mIconIDs,item,modifPair.second.paramName.c_str(),modifPair.second.paramDesc,artValue,2,0,-1,modifPair.first);
}
