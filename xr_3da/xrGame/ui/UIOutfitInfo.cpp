#include "StdAfx.h"
#include "UIOutfitInfo.h"
#include "UIStatic.h"
#include "UIScrollView.h"
#include "../actor.h"
#include "../CustomOutfit.h"
#include "../string_table.h"
#include "UIListItem.h"
#include "UIListWnd.h"
#include "UIListItemIconed.h"
#include "UIFrameLineWnd.h"
#include "UIXmlInit.h"
#include "../InventoryOwner.h"
#include "../entity_alive.h"
#include "../inventory.h"
#include "../Artifact.h"


CUIOutfitInfo::CUIOutfitInfo(): m_outfit(nullptr), m_list(nullptr)
{
	//immunes=OPFuncs::CreateImmunesStringMap();
	//restores=OPFuncs::CreateRestoresStringMap();
}

CUIOutfitInfo::~CUIOutfitInfo() {}

void CUIOutfitInfo::InitFromXml(CUIXml& xml_doc)
{
	LPCSTR _base				= "outfit_info";
	string256					_buff;
	CUIXmlInit::InitWindow		(xml_doc, _base, 0, this);

	m_list=xr_new<CUIListWnd>();
	m_list->SetAutoDelete(true);
	strconcat(sizeof(_buff),_buff, _base, ":immunities_list");
	xml_path=_buff;
	CUIXmlInit::InitListWnd(xml_doc,_buff,0,m_list);
	m_list->SetMessageTarget(this);
	m_list->EnableScrollBar(true);
	AttachChild(m_list);

	strconcat(sizeof(_buff),_buff, _base, ":immunities_list:icons");
	CUIXmlInit::GetStringTable(xml_doc,_buff,0,iconIDs);
}

void CUIOutfitInfo::createImmuneItem(CCustomOutfit* outfit,ALife::EHitType hitType, bool force_add)
{
	float _val_outfit			= outfit ? outfit->GetDefHitTypeProtection(ALife::EHitType(hitType)) : 1.0f;
	_val_outfit			= 1.0f - _val_outfit;
	float _val_af				= Actor()->HitArtefactsOnBelt(1.0f,ALife::EHitType(hitType));
	_val_af				= 1.0f - _val_af;
	bool emptyParam=fsimilar(_val_outfit, 0.0f) && fsimilar(_val_af, 0.0f) && !force_add;
	LPCSTR hitName= ALife::g_cafHitType2String(hitType);
	std::vector<CUIListItemIconed*>::iterator item_it=std::find_if(m_lImmuneUnsortedItems.begin(),m_lImmuneUnsortedItems.end(),[&](CUIListItem* item)
	{
		CUIListItemIconed* ii=smart_cast<CUIListItemIconed*>(item);
		if (!ii)
			return false;
		return ii->GetData()==(void*)hitName;
	});
	CUIListItemIconed *item;
	if (item_it==m_lImmuneUnsortedItems.end())
	{
		if (emptyParam)
			return;
		CUIXml uiXml;
		uiXml.Init(CONFIG_PATH, UI_PATH, "inventory_new.xml");
		item=xr_new<CUIListItemIconed>();
		CUIXmlInit::InitIconedColumns(uiXml,xml_path.c_str(),0,item);
		item->SetData((void*)hitName);
		item->SetAutoDelete(false);
		m_lImmuneUnsortedItems.push_back(item);
	}
	else 
	{
		if (emptyParam)
		{
			xr_delete(*item_it);
			m_lImmuneUnsortedItems.erase(std::remove(m_lImmuneUnsortedItems.begin(),m_lImmuneUnsortedItems.end(),*item_it),m_lImmuneUnsortedItems.end());
			return;
		}
		item=*item_it;
	}
	xr_map<shared_str ,shared_str>::iterator icon=iconIDs.find(hitName);
	if (icon!=iconIDs.end())
	{
		if (icon->second.size()>0)
			item->SetFieldIcon(0,icon->second.c_str());
	}
	
	string128 buff;
	sprintf_s(buff,"ui_inv_outfit_%s_protection",hitName);
	item->SetFieldText(1,CStringTable().translate(buff).c_str());

	string128 hint;
	sprintf_s(hint,"ui_inv_outfit_%s_protection_hint",hitName);
	if (CStringTable().IDExist(hint))
	{
		
		item->m_hint_text=CStringTable().translate(hint);
	}

	bool outfitPresent=false;
	if (!fsimilar(_val_outfit, 0.0f))
	{
		string128 buff_outfit;
		sprintf_s	(buff_outfit,"%s %+3.0f%%", (_val_outfit>0.0f)?"%c[green]":"%c[red]", _val_outfit*100.0f);
		item->SetFieldText(2,buff_outfit);
		outfitPresent=true;
	}
	item->SetVisibility(2,outfitPresent);

	bool artPresent=false;
	if( !fsimilar(_val_af, 0.0f) )
	{
		string128 buff_art;
		sprintf_s	(buff_art,"%s %+3.0f%%", (_val_af>0.0f)?"%c[green]":"%c[red]", _val_af*100.0f);
		item->SetFieldText(3,buff_art);
		artPresent=true;
	}
	item->SetVisibility(3,artPresent);
}

//float getRestoresFromBelt(int restoreId)
//{
//	if (!g_actor)
//		return 0;
//	float result=0;
//	std::for_each(Actor()->inventory().m_belt.begin(),Actor()->inventory().m_belt.end(),[&](CInventoryItem* item)
//	{
//		CArtefact*	artefact = smart_cast<CArtefact*>(item);
//		if(artefact)
//		{
//			switch(restoreId)
//			{
//				case 1:{}break;
//				case 2:{}break;
//				case 3:{}break;
//				case 4:{}break;
//				case 5:{}break;
//				default:
//					NODEFAULT;
//			}
//		}
//	});
//	return result;
//}

//void CUIOutfitInfo::createRestoreItem(CCustomOutfit* outfit, int restoreId, bool force_add)
//{
//	switch (restoreId)
//	{
//		case 0:
//		{
//			float outfitCondition=outfit?outfit->GetCondition():0;
//			float outfitBleedingValue= outfit ? outfit->m_fBleedingRestoreSpeed*outfit->GetCondition():0;
//
//		}
//		break;
//		case 1:
//		{
//			
//		}
//		break;
//		case 2:{}break;
//		case 3:{}break;
//		case 4:{}break;
//		case 5:{}break;
//		default:
//			NODEFAULT;
//	}
//}

void CUIOutfitInfo::Update(CCustomOutfit* outfitP)
{
	m_outfit				= outfitP;
	m_list->RemoveAll();
	std::for_each(immunes.begin(),immunes.end(),[&](std::pair<ALife::EHitType,shared_str> immunePair)
	{
		createImmuneItem(m_outfit,immunePair.first,false);
	});
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

//	std::for_each(restores.begin(),restores.end(),[](std::pair<int,OPFuncs::restoreParam> restoresPair)
//	{
		//createResoreItems();
//	});
	/*CUIListItem* separator=xr_new<CUIListItem>();
	separator->SetText("Модификаторы");
	separator->SetAutoDelete(true);
	m_list->AddItem(separator);*/
}

void CUIOutfitInfo::UpdateImmuneView()
{
	CEntityAlive *pEntityAlive = smart_cast<CEntityAlive*>(Level().CurrentEntity());
	CInventoryOwner* pOurInvOwner = smart_cast<CInventoryOwner*>(pEntityAlive);
	CCustomOutfit* pOutfit	= smart_cast<CCustomOutfit*>(pOurInvOwner->inventory().m_slots[OUTFIT_SLOT].m_pIItem);		
	Update(pOutfit);		
}



