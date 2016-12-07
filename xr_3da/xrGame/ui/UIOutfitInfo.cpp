#include "StdAfx.h"
#include "UIOutfitInfo.h"
#include "UIStatic.h"
#include "UIScrollView.h"
#include "../actor.h"
#include "../CustomOutfit.h"
#include "../string_table.h"
#include "UIListItemAdv.h"
#include "UIListItemIconed.h"
#include "UIXmlInit.h"
#include "../InventoryOwner.h"
#include "../entity_alive.h"
#include "../inventory.h"

CUIOutfitInfo::CUIOutfitInfo(): m_outfit(nullptr), m_list(nullptr) {}

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
	XML_NODE* iconsNode		= xml_doc.NavigateToNode(_buff,0);

	if (iconsNode)
	{
		for (XML_NODE* node=iconsNode->FirstChild(); node; node=node->NextSibling())
		{
			if (node)
			{	
				LPCSTR id=node->Value();
				LPCSTR value=nullptr;
				XML_NODE *data=node->FirstChild();
				if (data)
				{
					TiXmlText *text			= data->ToText();
					if (text)				
						value=text->Value();
					iconIDs.insert(mk_pair(id,value));

				}
			}
		}
	}
}

void CUIOutfitInfo::Update(CCustomOutfit* outfitP)
{
	m_outfit				= outfitP;
	NewSetItem(ALife::eHitTypeBurn,			false);
	NewSetItem(ALife::eHitTypeShock,		false);
	NewSetItem(ALife::eHitTypeStrike,		false);
	NewSetItem(ALife::eHitTypeWound,		false);
	NewSetItem(ALife::eHitTypeRadiation,	false);
	NewSetItem(ALife::eHitTypeTelepatic,	false);
	NewSetItem(ALife::eHitTypeChemicalBurn,	false);
	NewSetItem(ALife::eHitTypeExplosion,	false);
	NewSetItem(ALife::eHitTypeFireWound,	false);
}

void CUIOutfitInfo::UpdateImmuneView()
{
	CEntityAlive *pEntityAlive = smart_cast<CEntityAlive*>(Level().CurrentEntity());
	CInventoryOwner* pOurInvOwner = smart_cast<CInventoryOwner*>(pEntityAlive);
	CCustomOutfit* pOutfit	= smart_cast<CCustomOutfit*>(pOurInvOwner->inventory().m_slots[OUTFIT_SLOT].m_pIItem);		
	Update(pOutfit);		
}

void CUIOutfitInfo::NewSetItem(ALife::EHitType hitType, bool force_add)
{
	float _val_outfit			= m_outfit ? m_outfit->GetDefHitTypeProtection(ALife::EHitType(hitType)) : 1.0f;
	_val_outfit			= 1.0f - _val_outfit;
	float _val_af				= Actor()->HitArtefactsOnBelt(1.0f,ALife::EHitType(hitType));
	_val_af				= 1.0f - _val_af;

	bool emptyParam=fsimilar(_val_outfit, 0.0f) && fsimilar(_val_af, 0.0f) && !force_add;

	LPCSTR hitName= ALife::g_cafHitType2String(hitType);
	int itemIndex=m_list->FindItem((void*)hitName);
	CUIListItemIconed *item;
	if (itemIndex==-1)
	{
		if (emptyParam)
			return;
		CUIXml uiXml;
		uiXml.Init(CONFIG_PATH, UI_PATH, "inventory_new.xml");
		item=xr_new<CUIListItemIconed>();
		item->InitXml(xml_path.c_str(),uiXml);
		item->SetData((void*)hitName);
		item->SetAutoDelete(true);
		m_list->AddItem<CUIListItemIconed>(item);
	}
	else 
	{
		if (emptyParam)
		{
			m_list->RemoveItem(itemIndex);
			return;
		}
		item=static_cast<CUIListItemIconed*>(m_list->GetItem(itemIndex));
	}

	xr_map<shared_str ,shared_str>::iterator icon=iconIDs.find(hitName);
	if (icon!=iconIDs.end())
	{
		if (icon->second.size()>0)
			item->SetFieldIcon(0,icon->second.c_str());
	}
	
	string64 buff;
	sprintf_s(buff,"ui_inv_outfit_%s_protection",hitName);
	item->SetFieldText(1,CStringTable().translate(buff).c_str());
	bool outfit=false;
	if (!fsimilar(_val_outfit, 0.0f))
	{
		string64 buff_outfit;
		sprintf_s	(buff_outfit,"%s %+3.0f%%", (_val_outfit>0.0f)?"%c[green]":"%c[red]", _val_outfit*100.0f);
		item->SetFieldText(2,buff_outfit);
		outfit=true;
	}
	item->SetVisibility(2,outfit);

	bool art=false;
	if( !fsimilar(_val_af, 0.0f) )
	{
		string64 buff_art;
		sprintf_s	(buff_art,"%s %+3.0f%%", (_val_af>0.0f)?"%c[green]":"%c[red]", _val_af*100.0f);
		item->SetFieldText(3,buff_art);
		art=true;
	}
	item->SetVisibility(3,art);

	//incorrect sum values...
	/*string64 buff_total;
	float total=_val_outfit+ _val_af;
	sprintf_s	(buff_total,"%s %+3.0f%%", (total>0.0f)?"%c[green]":"%c[red]", total*100.0f);
	item->SetVisibility(4,outfit && art);
	item->SetFieldText(4,buff_total);*/
}
