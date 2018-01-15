#include "stdafx.h"

#include "uiiteminfo.h"
#include "uistatic.h"
#include "UIXmlInit.h"

#include "UIListWnd.h"
#include "UIProgressBar.h"
#include "UIScrollView.h"

#include "../string_table.h"
#include "../Inventory_Item.h"
#include "UIInventoryUtilities.h"
#include "../PhysicsShellHolder.h"
#include "UIDragDropListEx.h"
#include "UIWpnParams.h"
#include "../game_object_space.h"
#include "ui_af_params.h"
#include "../actor.h"
#include "../script_callback_ex.h"
#include "../CustomOutfit.h"
#include "../exooutfit.h"
#include "UICellItemFactory.h"

CUIItemInfo::CUIItemInfo(): m_pBatteryText(nullptr), m_pBatteryIconBackground(nullptr), m_pChargeBatteryProgress(nullptr)
{
	UIItemImageSize.set(0.0f, 0.0f);
	UICondProgresBar = nullptr;
	UICondition = nullptr;
	UICost = nullptr;
	UIWeight = nullptr;
	UIItemImage = nullptr;
	UIDesc = nullptr;
	UIWpnParams = nullptr;
	UIArtefactParams = nullptr;
	UIOutfitParams = nullptr;
	UIName = nullptr;
	m_pInvItem = nullptr;
	m_b_force_drawing = false;
	m_pBatteryIcon = nullptr;
}

CUIItemInfo::~CUIItemInfo()
{
	xr_delete					(UIWpnParams);
	xr_delete					(UIArtefactParams);
	xr_delete					(UIOutfitParams);
}

void CUIItemInfo::Init(LPCSTR xml_name){

	CUIXml                                          uiXml;
	bool xml_result				= uiXml.Init(CONFIG_PATH, UI_PATH, xml_name);
	R_ASSERT2					(xml_result, "xml file not found");

	CUIXmlInit					xml_init;

	if(uiXml.NavigateToNode("main_frame",0))
	{
		Frect wnd_rect;
		wnd_rect.x1		= uiXml.ReadAttribFlt("main_frame", 0, "x", 0);
		wnd_rect.y1		= uiXml.ReadAttribFlt("main_frame", 0, "y", 0);

		wnd_rect.x2		= uiXml.ReadAttribFlt("main_frame", 0, "width", 0);
		wnd_rect.y2		= uiXml.ReadAttribFlt("main_frame", 0, "height", 0);
		
		inherited::Init(wnd_rect.x1, wnd_rect.y1, wnd_rect.x2, wnd_rect.y2);
	}

	if(uiXml.NavigateToNode("static_name",0))
	{
		UIName						= xr_new<CUIStatic>();	 
		AttachChild					(UIName);		
		UIName->SetAutoDelete		(true);
		xml_init.InitStatic			(uiXml, "static_name", 0,	UIName);
	}
	if(uiXml.NavigateToNode("static_weight",0))
	{
		UIWeight				= xr_new<CUIStatic>();	 
		AttachChild				(UIWeight);		
		UIWeight->SetAutoDelete(true);
		xml_init.InitStatic		(uiXml, "static_weight", 0,			UIWeight);
	}
	if(uiXml.NavigateToNode("static_cost",0))
	{
		UICost					= xr_new<CUIStatic>();	 
		AttachChild				(UICost);
		UICost->SetAutoDelete	(true);
		xml_init.InitStatic		(uiXml, "static_cost", 0,			UICost);
	}
	if(uiXml.NavigateToNode("static_condition",0))
	{
		UICondition					= xr_new<CUIStatic>();	 
		AttachChild					(UICondition);
		UICondition->SetAutoDelete	(true);
		xml_init.InitStatic			(uiXml, "static_condition", 0,		UICondition);
	}
	if(uiXml.NavigateToNode("condition_progress",0))
	{
		UICondProgresBar			= xr_new<CUIProgressBar>(); 
		AttachChild(UICondProgresBar);
		UICondProgresBar->SetAutoDelete(true);
		xml_init.InitProgressBar	(uiXml, "condition_progress", 0, UICondProgresBar);
	}

	if(uiXml.NavigateToNode("descr_list",0))
	{
		UIWpnParams						= xr_new<CUIWpnParams>();
		UIWpnParams->InitFromXml		(uiXml);
		UIArtefactParams				= xr_new<CUIArtefactParams>();
		UIArtefactParams->InitFromXml	(uiXml);
		UIOutfitParams					= xr_new<CUIOutfitParams>();
		UIOutfitParams->InitFromXml		(uiXml);

		UIDesc							= xr_new<CUIScrollView>(); 
		AttachChild						(UIDesc);		
		UIDesc->SetAutoDelete			(true);
		m_desc_info.bShowDescrText		= !!uiXml.ReadAttribInt("descr_list",0,"only_text_info", 1);
		xml_init.InitScrollView			(uiXml, "descr_list", 0, UIDesc);
		xml_init.InitFont				(uiXml, "descr_list:font", 0, m_desc_info.uDescClr, m_desc_info.pDescFont);
	}	

	if (uiXml.NavigateToNode("image_static", 0))
	{	
		UIItemImage					= xr_new<CUIStatic>();	 
		AttachChild					(UIItemImage);	
		UIItemImage->SetAutoDelete	(true);
		xml_init.InitStatic			(uiXml, "image_static", 0, UIItemImage);
		UIItemImage->TextureAvailable(true);

		UIItemImage->TextureOff			();
		UIItemImage->ClipperOn			();
		UIItemImageSize.set				(UIItemImage->GetWidth(),UIItemImage->GetHeight());
	}

	if (uiXml.NavigateToNode("battery_icon", 0))
	{
		m_pBatteryIcon = xr_new<CUIExoBatteryStatic>();
		m_pBatteryIcon->SetAutoDelete(true);
		m_pBatteryIcon->SetShader(InventoryUtilities::GetEquipmentIconsShader());
		m_pBatteryIcon->TextureAvailable(false);
		m_pBatteryIcon->TextureOff();
		m_pBatteryIcon->SetParentItem(nullptr);
		CUIXmlInit().InitStatic(uiXml, "battery_icon", 0, m_pBatteryIcon);
		AttachChild(m_pBatteryIcon);
		m_pBatteryIconBackground = xr_new<CUIFrameWindow>();
		m_pBatteryIconBackground->SetAutoDelete(true);
		m_pBatteryIcon->AttachChild(m_pBatteryIconBackground);
		CUIXmlInit().InitFrameWindow(uiXml, "battery_icon:background", 0, m_pBatteryIconBackground);

		m_pChargeBatteryProgress = xr_new<CUIProgressBar>();
		m_pChargeBatteryProgress->SetAutoDelete(true);
		CUIXmlInit().InitProgressBar(uiXml, "battery_charge_progress", 0, m_pChargeBatteryProgress);
		AttachChild(m_pChargeBatteryProgress);
		m_pChargeBatteryProgress->SetProgressPos(0);

		m_pBatteryText = xr_new<CUIStatic>();
		m_pBatteryText->SetAutoDelete(true);
		CUIXmlInit().InitStatic(uiXml, "battery_charge_text", 0, m_pBatteryText);
		AttachChild(m_pBatteryText);
	}
	xml_init.InitAutoStaticGroup	(uiXml, "auto", 0, this);
}

void CUIItemInfo::Init(float x, float y, float width, float height, LPCSTR xml_name)
{
	inherited::Init	(x, y, width, height);
	Init			(xml_name);
}

bool				IsGameTypeSingle();

void CUIItemInfo::InitItem(CInventoryItem* pInvItem)
{
	m_pInvItem				= pInvItem;
	if(!m_pInvItem)
	{
		if (UICondition)
			UICondition->Show(false);
		if(UICondProgresBar)
			UICondProgresBar->Show(false);
		return;
	}

	string256				str;
	if(UIName)
	{
		UIName->SetText		(pInvItem->Name());
	}
	if(UIWeight)
	{
		sprintf_s				(str, "%3.2f kg", pInvItem->Weight());
		UIWeight->SetText	(str);
	}
	if( UICost && IsGameTypeSingle() )
	{
		sprintf_s				(str, "%d %s", pInvItem->Cost(),*CStringTable().translate("ui_st_money_regional"));		// will be owerwritten in multiplayer
		UICost->SetText		(str);
	}
	if(UICondProgresBar)
	{
		float cond							= pInvItem->GetConditionToShow();
		UICondProgresBar->Show				(true);
		UICondProgresBar->SetProgressPos	( cond*100.0f+1.0f-EPS );
	}
	if (UICondition)
		UICondition->Show(true);
	if(UIDesc)
	{
		UIDesc->Clear						();
		VERIFY								(0==UIDesc->GetSize());
		TryAddWpnInfo						(pInvItem->object().cNameSect());
		TryAddArtefactInfo					(pInvItem->object().cNameSect());
		TryAddOutfitInfo					(pInvItem);
		if (pInvItem->GetAPRadiation()!=0)
		{
			float actorVal = pSettings->r_float("actor_condition", "radiation_v");
			float value = pInvItem->GetAPRadiation() / actorVal;
			CUIStatic* apRad = xr_new<CUIStatic>();
			apRad->SetFont(m_desc_info.pDescFont);
			apRad->SetWidth(UIDesc->GetDesiredChildWidth());
			string256 buf;
			LPCSTR color = (value > 0) ? "%c[red]" : "%c[green]";
			LPCSTR units = CStringTable().translate("ui_inv_radiation_units").c_str();
			LPCSTR desc= CStringTable().translate("ui_inv_radiation_irr").c_str();
			sprintf_s(buf, "%s %s%+3.2f%s", desc, color, value, units);
			apRad->SetText(buf);
			apRad->SetTextComplexMode(true);
			apRad->AdjustHeightToText();
			UIDesc->AddWindow(apRad, true);
		}
		if (Actor())
			Actor()->callback(GameObject::ECallbackType::OnPrepareItemInfo)(UIDesc);
		
		if(m_desc_info.bShowDescrText)
		{
			CUIStatic* pItem					= xr_new<CUIStatic>();
			pItem->SetTextColor					(m_desc_info.uDescClr);
			pItem->SetFont						(m_desc_info.pDescFont);
			pItem->SetWidth						(UIDesc->GetDesiredChildWidth());
			pItem->SetTextComplexMode			(true);
			pItem->SetText						(*pInvItem->GetItemDescription());
			pItem->AdjustHeightToText			();
			UIDesc->AddWindow					(pItem, true);
		}
		UIDesc->ScrollToBegin				();
	}
	if(UIItemImage)
	{
		// Загружаем картинку
		UIItemImage->SetShader				(InventoryUtilities::GetEquipmentIconsShader());

		Frect rect = pInvItem->GetIconInfo().getOriginalRect();
		UIItemImage->GetUIStaticItem().SetOriginalRect(rect);
		UIItemImage->TextureOn				();
		UIItemImage->ClipperOn				();
		UIItemImage->SetStretchTexture		(true);
		Frect v_r							= {	0.0f, 
												0.0f, 
												rect.width(),rect.height()};
		if(UI()->is_16_9_mode())
			v_r.x2 /= 1.328f;

		UIItemImage->GetUIStaticItem().SetRect	(v_r);
		UIItemImage->SetWidth					(_min(v_r.width(),	UIItemImageSize.x));
		UIItemImage->SetHeight					(_min(v_r.height(),	UIItemImageSize.y));
	}

	if (m_pBatteryIcon)
	{
		m_pBatteryIcon->Show(false);
		m_pBatteryText->Show(false);
		m_pChargeBatteryProgress->Show(false);
		m_pChargeBatteryProgress->SetProgressPos(0);
		m_pBatteryIcon->TextureOff();
		m_pBatteryIcon->TextureAvailable(false);
		CExoOutfit* exo = smart_cast<CExoOutfit*>(m_pInvItem);
		if (exo && exo->BatteryAccepted())
		{
			if (exo->m_sCurrentBattery.size() > 0)
			{
				UIIconInfo iconInfo(exo->m_sCurrentBattery);
				m_pBatteryIcon->SetOriginalRect(iconInfo.getOriginalRect());
				m_pBatteryIcon->SetStretchTexture(true);
				m_pBatteryIcon->TextureOn();
				m_pBatteryIcon->TextureAvailable(true);
				m_pChargeBatteryProgress->SetProgressPos(exo->m_fCurrentCharge*100.0f + 1.0f - EPS);
			}
			m_pBatteryIcon->Show(true);
			m_pBatteryText->Show(true);			
			m_pChargeBatteryProgress->Show(true);
		}

	}
}

void CUIItemInfo::TryAddWpnInfo (const shared_str& wpn_section){
	if (UIWpnParams->Check(wpn_section))
	{
		UIWpnParams->SetInfo(wpn_section,UIDesc);
	}
}

void CUIItemInfo::TryAddArtefactInfo	(const shared_str& af_section)
{
	if (UIArtefactParams->Check(af_section))
	{
		UIArtefactParams->SetInfo(af_section,UIDesc);
	}
}

void CUIItemInfo::TryAddOutfitInfo(CInventoryItem* outfitItem)
{
	if (UIOutfitParams->Check(outfitItem))
	{
		UIOutfitParams->SetInfo(smart_cast<CCustomOutfit*>(outfitItem),UIDesc);
	}
}

void CUIItemInfo::Draw()
{
	if(m_pInvItem || m_b_force_drawing)
		inherited::Draw();
}
