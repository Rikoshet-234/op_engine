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
#include "../ActorCondition.h"
#include "../InventoryOwner.h"
#include "../entity_alive.h"
#include "../inventory.h"
#include "../Artifact.h"
#include "../OPFuncs/utils.h"

CUIOutfitInfo::CUIOutfitInfo(): m_outfit(nullptr), m_bShowModifiers(false), m_list(nullptr)
{
	immunes = OPFuncs::CreateImmunesStringMap();
	modificators = OPFuncs::CreateRestoresStringMap();
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
	m_bShowModifiers=xml_doc.ReadAttribInt(_buff,0,"show_modifiers",0)==1?true:false;
	strconcat(sizeof(_buff),_buff, _base, ":immunities_list:icons");
	CUIXmlInit::GetStringTable(xml_doc,_buff,0,iconIDs);
}

void CUIOutfitInfo::ClearAll()
{
	ClearItems(m_lImmuneUnsortedItems);
	ClearItems(m_lModificatorsItems);
}

void CUIOutfitInfo::ClearItems(std::vector<CUIListItemIconed*> &basedList)
{
	while(!basedList.empty())
	{
		auto item=basedList.front();
		basedList.erase(std::remove(basedList.begin(),basedList.end(),item),basedList.end());
		item->DetachAll();
		xr_delete(item);
	}
}

CUIListItemIconed* findIconedItem(std::vector<CUIListItemIconed*> &basedList,LPCSTR keyValue,bool emptyParam,LPCSTR xml_path)
{
	std::vector<CUIListItemIconed*>::iterator item_it=std::find_if(basedList.begin(),basedList.end(),[&](CUIListItem* item)
	{
		CUIListItemIconed* ii=smart_cast<CUIListItemIconed*>(item);
		if (!ii)
			return false;
		return ii->GetData()==(void*)keyValue;
	});
	CUIListItemIconed *item;
	if (item_it==basedList.end())
	{
		if (emptyParam)
			return nullptr;
		CUIXml uiXml;
		uiXml.Init(CONFIG_PATH, UI_PATH, "inventory_new.xml");
		item=xr_new<CUIListItemIconed>();
		CUIXmlInit::InitIconedColumns(uiXml,xml_path,0,item);
		item->SetData((void*)keyValue);
		item->SetAutoDelete(false);
		basedList.push_back(item);
	}
	else 
	{
		if (emptyParam)
		{
			xr_delete(*item_it);
			basedList.erase(std::remove(basedList.begin(),basedList.end(),*item_it),basedList.end());
			return nullptr;
		}
		item=*item_it;
	}
	return item;
}

void setIconedItem(xr_map<shared_str ,shared_str> iconIDs,CUIListItemIconed* item,LPCSTR iconKey,shared_str column1Value,float column2Value,int column2Type,float column3Value,int column3Type,int addParam=0)
{
	xr_map<shared_str ,shared_str>::iterator icon=iconIDs.find(iconKey);
	if (icon!=iconIDs.end())
	{
		if (icon->second.size()>0)
			item->SetFieldIcon(0,icon->second.c_str());
	}
	item->SetFieldText(1,CStringTable().translate(column1Value).c_str());
	/*string128 hint;
	sprintf_s(hint,"%s_hint",column1Value.c_str());
	if (CStringTable().IDExist(hint))
		item->m_hint_text=CStringTable().translate(hint);*/
	bool outfitPresent=false;
	if (!fsimilar(column2Value, 0.0f))
	{
		string128 buff_outfit;
		switch(column2Type)
		{
		case 0:
			sprintf_s	(buff_outfit,"%s%+3.0f%%", (column2Value>0.0f)?"%c[green]":"%c[red]", column2Value*100.0f);
			break;
		case 1:
			{
				LPCSTR color = (column2Value<0)?"%c[red]":"%c[green]";
				if ((column2Value>0 && column2Value<1) || (column2Value<0 && column2Value>-1))
				{
					column2Value=column2Value*1000;
					sprintf_s	(buff_outfit,"%s%+3.0f%s", color, column2Value,CStringTable().translate("ui_inv_aw_gr").c_str());
				}
				else
					sprintf_s	(buff_outfit,"%s%+3.0f%s", color, column2Value,CStringTable().translate("ui_inv_aw_kg").c_str());
			}
			break;
		case 2:
			{
				LPCSTR color=(column2Value>0.0f)?"%c[green]":"%c[red]";
				if (addParam==BLEEDING_RESTORE_ID||addParam==RADIATION_RESTORE_ID)
					color = (column2Value>0)?"%c[red]":"%c[green]";
				if (column2Value>9999)
				{
					column2Value/=10000;
					sprintf_s	(buff_outfit,"%s%+3.0fk%%", color, column2Value);
				}
				else
					sprintf_s	(buff_outfit,"%s%+3.0f%%", color, column2Value);
			}
			break;
		default:NODEFAULT;
		}
		item->SetFieldText(2,buff_outfit);
		outfitPresent=true;
	}
	item->SetVisibility(2,outfitPresent);
	bool artPresent=false;
	if( !fsimilar(column3Value, 0.0f) )
	{
		string128 buff_art;
		switch(column3Type)
		{
		case 0:
			sprintf_s	(buff_art,"%s%+3.0f%%", (column3Value>0.0f)?"%c[green]":"%c[red]", column3Value*100.0f);
			break;
		case 1:
			{
				LPCSTR color = (column3Value<0)?"%c[red]":"%c[green]";
				if ((column3Value>0 && column3Value<1) || (column3Value<0 && column3Value>-1))
				{
					column3Value=column3Value*1000;
					sprintf_s	(buff_art,"%s%+3.0f%s", color, column3Value,CStringTable().translate("ui_inv_aw_gr").c_str());
				}
				else
					sprintf_s	(buff_art,"%s%+3.0f%s", color, column3Value,CStringTable().translate("ui_inv_aw_kg").c_str());
			}
			break;
		case 2:
			{
				LPCSTR color=(column3Value>0.0f)?"%c[green]":"%c[red]";
				if (addParam==BLEEDING_RESTORE_ID||addParam==RADIATION_RESTORE_ID)
					color = (column3Value>0)?"%c[red]":"%c[green]";
				if (column3Value>9999)
				{
					column3Value/=1000;
					sprintf_s	(buff_art,"%s%+3.0fk%%", color, column3Value);
				}
				else
					sprintf_s	(buff_art,"%s%+3.0f%%", color, column3Value);
			}
			break;
		default:NODEFAULT;
		}
		item->SetFieldText(3,buff_art);
		artPresent=true;
	}
	item->SetVisibility(3,artPresent);
}

void addSeparator(CUIListWnd* list,shared_str textId)
{
		CUIListItem* separator=xr_new<CUIListItem>();
		separator->SetAutoDelete(true);
		separator->SetText(CStringTable().translate(textId).c_str());
		separator->SetTextAlignment(ETextAlignment::alCenter);
		list->AddItem(separator);
}

void addSeparatorWT(CUIListWnd* list)
{
		CUIListItem* separator=xr_new<CUIListItem>();
		separator->SetAutoDelete(true);
		separator->SetHeight(5);
		list->AddItem(separator);
}

void CUIOutfitInfo::createImmuneItem(CCustomOutfit* outfit,std::pair<ALife::EHitType,shared_str> immunePair, bool force_add)
{
	float _val_outfit			= outfit ? outfit->GetDefHitTypeProtection(ALife::EHitType(immunePair.first)) : 1.0f;
	_val_outfit			= 1.0f - _val_outfit;
	float _val_af				= Actor()->HitArtefactsOnBelt(1.0f,ALife::EHitType(immunePair.first));
	_val_af				= 1.0f - _val_af;
	bool emptyParam=fsimilar(_val_outfit, 0.0f) && fsimilar(_val_af, 0.0f) && !force_add;
	LPCSTR hitName= ALife::g_cafHitType2String(immunePair.first);
	CUIListItemIconed* item=findIconedItem(m_lImmuneUnsortedItems,hitName,emptyParam,xml_path.c_str());
	if (!item)
		return;
	setIconedItem(iconIDs,item,hitName,immunePair.second,_val_outfit,0,_val_af,0);
}

void CUIOutfitInfo::createModifItem(CCustomOutfit* outfit,std::pair<int, OPFuncs::restoreParam> modifPair, bool force_add)
{
	float outfitValue=0;
	float artsValue=artefactRestores[modifPair.first];
	switch (modifPair.first)
	{
		case BLEEDING_RESTORE_ID:
			{
				float actorVal= pSettings->r_float	("actor_condition", "wound_incarnation_v");
				artsValue=(artsValue/actorVal)*100*-1;
				outfitValue = outfit ? outfit->m_fBleedingRestoreSpeed*outfit->GetCondition():0;
				outfitValue=outfitValue*1000*-1;
			}
			break;
		case SATIETY_RESTORE_ID:
			{
				float actorVal= pSettings->r_float	("actor_condition", "satiety_v");
				artsValue=(artsValue/actorVal)*100;
				outfitValue = outfit ? outfit->m_fSatietyRestoreSpeed*outfit->GetCondition():0;
				outfitValue=outfitValue*1000;
			}
			break;
		case RADIATION_RESTORE_ID:
			{
				float actorVal= pSettings->r_float	("actor_condition", "radiation_v");
				artsValue=(artsValue/actorVal);
				outfitValue = outfit ? outfit->m_fRadiationRestoreSpeed*outfit->GetCondition():0;
				outfitValue=outfitValue*1000;
			}
			break;
		case HEALTH_RESTORE_ID:
			{
				float actorVal= pSettings->r_float	("actor_condition", "satiety_health_v");
				artsValue=(artsValue/actorVal)*100;
				outfitValue = outfit ? outfit->m_fHealthRestoreSpeed*outfit->GetCondition():0;
				outfitValue=outfitValue*1000;
			}
			break;
		case POWER_RESTORE_ID:
			{
				float actorVal= pSettings->r_float	("actor_condition", "satiety_power_v");
				artsValue=(artsValue/actorVal);
				outfitValue = outfit ? outfit->m_fPowerRestoreSpeed*outfit->GetCondition():0;
				outfitValue=outfitValue*1000;
			}
			break;
		case POWER_LOSS_ID:
			{
				outfitValue = outfit ? outfit->GetPowerLoss()*outfit->GetCondition():0;
				outfitValue=outfitValue*100;
			}
			break;
		default:
			NODEFAULT;
	}
	bool emptyParam=fsimilar(outfitValue, 0.0f) && fsimilar(artsValue, 0.0f) && !force_add;
	CUIListItemIconed* item=findIconedItem(m_lModificatorsItems,modifPair.second.paramName.c_str(),emptyParam,xml_path.c_str());
	if (!item)
		return;
	setIconedItem(iconIDs,item,modifPair.second.paramName.c_str(),modifPair.second.paramDesc,outfitValue,2,artsValue,2,modifPair.first);
}

void CUIOutfitInfo::Update(CCustomOutfit* outfitP)
{
	m_outfit				= outfitP;
	m_list->RemoveAll();
#pragma region update immune lines
	std::for_each(immunes.begin(),immunes.end(),[&](std::pair<ALife::EHitType,shared_str> immunePair)
	{
		createImmuneItem(m_outfit,immunePair,false);
	});
	if (m_lImmuneUnsortedItems.size()>0)
	{
		//addSeparator(m_list,"ui_st_params");
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
	if (m_bShowModifiers)
	{
#pragma region update modifier lines
		float outfitAddWeight=m_outfit ? m_outfit->m_additional_weight*m_outfit->GetCondition() : 0;
		float artefactsWeight=g_actor ? Actor()->GetArtefactAdditionalWeight(): 0;
		CUIListItemIconed* weightItem=findIconedItem(m_lModificatorsItems,"additional_weight",fsimilar(outfitAddWeight, 0.0f) && fsimilar(artefactsWeight, 0.0f),xml_path.c_str());
		if (weightItem)
			setIconedItem(iconIDs,weightItem,"additional_weight","ui_inv_outfit_additional_inventory_weight",outfitAddWeight,1,artefactsWeight,1);
		std::for_each(modificators.begin(),modificators.end(),[&](std::pair<int, OPFuncs::restoreParam> modifPair)
		{
			createModifItem(m_outfit,modifPair,false);
		});
		if (m_lModificatorsItems.size()>0)
		{
			//addSeparator(m_list,"ui_st_modifiers");
			addSeparatorWT(m_list);
			std::sort(m_lModificatorsItems.begin(),m_lModificatorsItems.end(),[](CUIListItem* i1, CUIListItem* i2)
			{
				CUIListItemIconed *iconedItem1=smart_cast<CUIListItemIconed*>(i1);
				CUIListItemIconed *iconedItem2=smart_cast<CUIListItemIconed*>(i2);
				if (!iconedItem1 || !iconedItem2)
					return false;
				return		lstrcmpi(iconedItem1->GetFieldText(1),iconedItem2->GetFieldText(1))<0;
			});
			std::for_each(m_lModificatorsItems.begin(),m_lModificatorsItems.end(),[&](CUIListItemIconed* item)
			{
				m_list->AddItem<CUIListItemIconed>(item);
			});
		}
#pragma endregion
	}
}

void CUIOutfitInfo::UpdateImmuneView()
{
	CEntityAlive *pEntityAlive = smart_cast<CEntityAlive*>(Level().CurrentEntity());
	CInventoryOwner* pOurInvOwner = smart_cast<CInventoryOwner*>(pEntityAlive);
	CCustomOutfit* pOutfit	= smart_cast<CCustomOutfit*>(pOurInvOwner->inventory().m_slots[OUTFIT_SLOT].m_pIItem);	
	if (g_actor)
	{
		artefactRestores.clear();//собираем информацию о модификаторах с артефактов
		for(u32 i=0;i<modificators.size();i++) 
			artefactRestores.push_back(0);
		std::for_each(Actor()->inventory().m_belt.begin(),Actor()->inventory().m_belt.end(),[&](CInventoryItem* item)
		{
			CArtefact*	artefact = smart_cast<CArtefact*>(item);
			if(artefact)
			{
				artefactRestores[0]+=artefact->m_fBleedingRestoreSpeed;
				artefactRestores[1]+=artefact->m_fSatietyRestoreSpeed;
				artefactRestores[2]+=artefact->m_fRadiationRestoreSpeed;
				artefactRestores[3]+=artefact->m_fHealthRestoreSpeed;
				artefactRestores[4]+=artefact->m_fPowerRestoreSpeed;
			}
		});
	}
	Update(pOutfit);		
}



