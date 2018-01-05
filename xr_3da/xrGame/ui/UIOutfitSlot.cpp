#include "stdafx.h"
#include "UIOutfitSlot.h"
#include "UIStatic.h"
#include "UICellItem.h"
#include "../CustomOutfit.h"
#include "../actor.h"
#include "UIInventoryUtilities.h"
#include "UIXmlInit.h"
#include "../exooutfit.h"
#include "UICellItemFactory.h"

CUIOutfitDragDropList::CUIOutfitDragDropList()
{
	m_background = xr_new<CUIStatic>();
	m_background->SetAutoDelete(true);
	CUIWindow::AttachChild(m_background);

	m_pBatteryIcon = xr_new<CUIDragDropListEx>();
	m_pBatteryIcon->SetAutoDelete(true);
	CUIWindow::AttachChild(m_pBatteryIcon);

	m_pChargeBatteryProgress= xr_new<CUIProgressBar>();
	m_pChargeBatteryProgress->SetAutoDelete(true);
	CUIWindow::AttachChild(m_pChargeBatteryProgress);
	m_pChargeBatteryProgress->SetProgressPos(0);

	m_pBatteryText=xr_new<CUIStatic>();
	m_pBatteryText->SetAutoDelete(true);
	CUIWindow::AttachChild(m_pBatteryText);

	m_bDrawBatteryPart = false;
	m_default_outfit			= "npc_icon_without_outfit";
}

CUIOutfitDragDropList::~CUIOutfitDragDropList()
{
}

#include "../level.h"
#include "../game_base_space.h"

void CUIOutfitDragDropList::SetOutfit(CUICellItem* itm)
{
	m_background->Init					(0,0, GetWidth(), GetHeight());
	m_background->SetStretchTexture		(true);
	m_bDrawBatteryPart=false;
	m_pBatteryIcon->ClearAll(true);
	m_pChargeBatteryProgress->SetProgressPos(0);
	if(itm)
	{
		PIItem _iitem	= static_cast<PIItem>(itm->m_pData);
		CCustomOutfit* pOutfit = smart_cast<CCustomOutfit*>(_iitem); VERIFY(pOutfit);
	
		m_background->InitTexture			(pOutfit->GetFullIconName().c_str());
		CExoOutfit* exo = smart_cast<CExoOutfit*>(pOutfit);
		if (exo && exo->BatteryAccepted())
		{
			if (exo->m_sCurrentBattery.size() > 0)
			{
				m_pBatteryIcon->SetItem(create_cell_item(exo->m_sCurrentBattery));
				m_pChargeBatteryProgress->SetProgressPos(exo->m_fCurrentCharge*100.0f + 1.0f - EPS);
			}
			m_bDrawBatteryPart=true;
		}
	}else
	{
		m_background->InitTexture		("npc_icon_without_outfit");
	}
	m_background->TextureAvailable		(true);
	m_background->TextureOn				();
}

void CUIOutfitDragDropList::SetDefaultOutfit(LPCSTR default_outfit){
	m_default_outfit = default_outfit;
}

void CUIOutfitDragDropList::UIInitBattery(CUIXml& xml_doc, const char* rootPath)
{
	string256 path;
	sprintf_s(path, "%s:%s", rootPath, "battery_icon");
	CUIXmlInit().InitDragDropListEx(xml_doc, path, 0, m_pBatteryIcon);
	sprintf_s(path, "%s:%s", rootPath, "battery_charge_progress");
	CUIXmlInit().InitProgressBar(xml_doc, path, 0, m_pChargeBatteryProgress);
	sprintf_s(path, "%s:%s", rootPath, "battery_charge_text");
	CUIXmlInit().InitStatic(xml_doc, path, 0, m_pBatteryText);
}

void CUIOutfitDragDropList::SetItem(CUICellItem* itm)
{
	if(itm)	inherited::SetItem			(itm);
	SetOutfit							(itm);
}

void CUIOutfitDragDropList::SetItem(CUICellItem* itm, Fvector2 abs_pos)
{
	if(itm)	inherited::SetItem			(itm, abs_pos);
	SetOutfit							(itm);
}

void CUIOutfitDragDropList::SetItem(CUICellItem* itm, Ivector2 cell_pos)
{
	if(itm)	inherited::SetItem			(itm, cell_pos);
	SetOutfit							(itm);
}

CUICellItem* CUIOutfitDragDropList::RemoveItem(CUICellItem* itm, bool force_root)
{
	VERIFY								(!force_root);
	CUICellItem* ci						= inherited::RemoveItem(itm, force_root);
	SetOutfit							(nullptr);
	return								ci;
}


void CUIOutfitDragDropList::Draw()
{
	m_background->Draw					();
	if (m_bDrawBatteryPart)
	{
		m_pBatteryIcon->Draw();
		m_pChargeBatteryProgress->Draw();
		m_pBatteryText->Draw();
	}
//.	inherited::Draw						();
}