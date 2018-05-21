#include "stdafx.h"
#include "UITradeWnd.h"
#include "../../defines.h"
#include "xrUIXmlParser.h"
#include "UIXmlInit.h"

#include "../Entity.h"
#include "../Weapon.h"
#include "../HUDManager.h"
#include "../WeaponAmmo.h"
#include "../Actor.h"
#include "../Trade.h"
#include "../UIGameSP.h"
#include "UIInventoryUtilities.h"
#include "../inventoryowner.h"
#include "UIPropertiesBox.h"
#include "../inventory.h"
#include "../level.h"
#include "../string_table.h"
#include "../character_info.h"
#include "UIMultiTextStatic.h"
#include "UIListBoxItem.h"
#include "UIItemInfo.h"
#include "../script_callback_ex.h"
#include "../game_object_space.h"
#include "UICharacterInfo.h"
#include "UIDragDropListEx.h"
#include "UICellItem.h"
#include "UICellItemFactory.h"
#include "../OPFuncs/utils.h"
#include "UIInventoryWnd.h"
#include "../medkit.h"
#include "../antirad.h"
#include "../bottleitem.h"
#include "../gbox.h"
#include "../exooutfit.h"


#define				TRADE_XML			"trade.xml"
#define				TRADE_CHARACTER_XML	"trade_character.xml"
#define				TRADE_ITEM_XML		"trade_item.xml"


CUITradeWnd::CUITradeWnd() : bStarted(false), m_bDealControlsVisible(false), m_pTrade(nullptr), m_pOthersTrade(nullptr)
{
	m_pCurrentCellItem = nullptr;
	CUITradeWnd::Init();
	CUITradeWnd::Hide();
	SetUIWindowType(EAWindowType::wtTrade);
}

CUITradeWnd::~CUITradeWnd()
{
	m_pCurrentCellItem = nullptr;
	std::for_each(sourceDragDropLists.begin(),sourceDragDropLists.end(),[](CUIDragDropListEx* list){list->ClearAll(true);});
}

void CUITradeWnd::re_init()
{
	sourceDragDropLists.clear();
	UICharacterInfoLeft.DetachAll();
	UICharacterInfoRight.DetachAll();
	UIOurIcon.DetachAll();
	UIOthersIcon.DetachAll();
	UIOurBagWnd.DetachAll();
	UIOthersBagWnd.DetachAll();
	UIOurTradeWnd.DetachAll();
	UIOthersTradeWnd.DetachAll();
	UIItemInfo.DetachAll();
	UIDescWnd.DetachAll();
	UIPropertiesBox.RemoveAll();
	DetachAll();
	Init();
}

void CUITradeWnd::Init()
{
	CUIXml								uiXml;
	bool xml_result						= uiXml.Init(CONFIG_PATH, UI_PATH, TRADE_XML);
	R_ASSERT3							(xml_result, "xml file not found", TRADE_XML);
	CUIXmlInit							xml_init;

	xml_init.InitWindow					(uiXml, "main", 0, this);
#pragma region common ui elements
	//статические элементы интерфейса
	AttachChild							(&UIStaticTop);
	xml_init.InitStatic					(uiXml, "top_background", 0, &UIStaticTop);
	AttachChild							(&UIStaticBottom);
	xml_init.InitStatic					(uiXml, "bottom_background", 0, &UIStaticBottom);

	//иконки с изображение нас и партнера по торговле
	AttachChild							(&UIOurIcon);
	xml_init.InitStatic					(uiXml, "static_icon", 0, &UIOurIcon);
	AttachChild							(&UIOthersIcon);
	xml_init.InitStatic					(uiXml, "static_icon", 1, &UIOthersIcon);
	UIOurIcon.AttachChild		(&UICharacterInfoLeft);
	UICharacterInfoLeft.Init	(0,0, UIOurIcon.GetWidth(), UIOurIcon.GetHeight(), TRADE_CHARACTER_XML);
	UIOthersIcon.AttachChild	(&UICharacterInfoRight);
	UICharacterInfoRight.Init	(0,0, UIOthersIcon.GetWidth(), UIOthersIcon.GetHeight(), TRADE_CHARACTER_XML);


	//Списки торговли
	AttachChild							(&UIOurBagWnd);
	xml_init.InitStatic					(uiXml, "our_bag_static", 0, &UIOurBagWnd);
	AttachChild							(&UIOthersBagWnd);
	xml_init.InitStatic					(uiXml, "others_bag_static", 0, &UIOthersBagWnd);

	UIOurBagWnd.AttachChild	(&UIOurMoneyStatic);
	xml_init.InitStatic					(uiXml, "our_money_static", 0, &UIOurMoneyStatic);

	UIOthersBagWnd.AttachChild(&UIOtherMoneyStatic);
	xml_init.InitStatic					(uiXml, "other_money_static", 0, &UIOtherMoneyStatic);

	AttachChild							(&UIOurTradeWnd);
	xml_init.InitStatic					(uiXml, "static_our_trade", 0, &UIOurTradeWnd);
	AttachChild							(&UIOthersTradeWnd);
	xml_init.InitStatic					(uiXml, "static_other_trade", 0, &UIOthersTradeWnd);

	UIOurTradeWnd.AttachChild	(&UIOurPriceCaption);
	xml_init.InitMultiTextStatic		(uiXml, "price_mt_static", 0, &UIOurPriceCaption);

	UIOthersTradeWnd.AttachChild(&UIOthersPriceCaption);
	xml_init.InitMultiTextStatic		(uiXml, "price_mt_static", 0, &UIOthersPriceCaption);

	//Списки Drag&Drop
	UIOurBagWnd.AttachChild	(&UIOurBagList);	
	xml_init.InitDragDropListEx			(uiXml, "dragdrop_list_our_bag", 0, &UIOurBagList);
	UIOurBagList.SetUIListId(IWListTypes::ltTradeOurBag);
	sourceDragDropLists.push_back(&UIOurBagList);

	UIOthersBagWnd.AttachChild(&UIOthersBagList);	
	xml_init.InitDragDropListEx			(uiXml, "dragdrop_list_other_bag", 0, &UIOthersBagList);
	UIOthersBagList.SetUIListId(IWListTypes::ltTradeOtherBag);
	sourceDragDropLists.push_back(&UIOthersBagList);


	UIOurTradeWnd.AttachChild	(&UIOurTradeList);	
	xml_init.InitDragDropListEx			(uiXml, "dragdrop_list_our_trade", 0, &UIOurTradeList);
	UIOurTradeList.SetUIListId(IWListTypes::ltTradeOurTrade);
	sourceDragDropLists.push_back(&UIOurTradeList);


	UIOthersTradeWnd.AttachChild(&UIOthersTradeList);	
	xml_init.InitDragDropListEx			(uiXml, "dragdrop_list_other_trade", 0, &UIOthersTradeList);
	UIOthersTradeList.SetUIListId(IWListTypes::ltTradeOtherTrade);
	sourceDragDropLists.push_back(&UIOthersTradeList);

	std::for_each(sourceDragDropLists.begin(),sourceDragDropLists.end(),[this](CUIDragDropListEx* list){BindDragDropListEvents(list);});

	
	AttachChild							(&UIDescWnd);
	xml_init.InitStatic					(uiXml, "desc_static", 0, &UIDescWnd);
	UIDescWnd.AttachChild		(&UIItemInfo);
	UIItemInfo.Init			(0,0, UIDescWnd.GetWidth(), UIDescWnd.GetHeight(), TRADE_ITEM_XML);


	xml_init.InitAutoStatic				(uiXml, "auto_static", this);


	AttachChild							(&UIPerformTradeButton);
	xml_init.Init3tButton					(uiXml, "button_perform_trade", 0, &UIPerformTradeButton);

	AttachChild							(&UIToTalkButton);
	xml_init.Init3tButton					(uiXml, "button_to_talk", 0, &UIToTalkButton);

	UIDealMsg					= nullptr;

	AttachChild							(&UIPropertiesBox);
	UIPropertiesBox.Init				(0,0,300,300);
	UIPropertiesBox.Hide				();
#pragma endregion

	uiXml.SetLocalRoot					(uiXml.NavigateToNode		("action_sounds",0));

	if (LPCSTR data=uiXml.Read("snd_open",		0,	nullptr))
		::Sound->create						(sounds[eInvSndOpen],data,st_Effect,sg_SourceType);
	if (LPCSTR data=uiXml.Read("snd_close",		0,	nullptr))
		::Sound->create						(sounds[eInvSndClose],data,st_Effect,sg_SourceType);
	if (LPCSTR data=uiXml.Read("snd_properties",		0,	nullptr))
		::Sound->create						(sounds[eInvProperties],data,st_Effect,sg_SourceType);
	if (LPCSTR data=uiXml.Read("snd_drop_item",		0,	nullptr))
		::Sound->create						(sounds[eInvDropItem],data,st_Effect,sg_SourceType);
	if (LPCSTR data=uiXml.Read("snd_detach_addon",		0,	nullptr))
		::Sound->create						(sounds[eInvDetachAddon],data,st_Effect,sg_SourceType);
	if (LPCSTR data=uiXml.Read("snd_item_use",		0,	nullptr))
		::Sound->create						(sounds[eInvItemUse],data,st_Effect,sg_SourceType);
	if (LPCSTR data=uiXml.Read("snd_trade_done",		0,	nullptr))
		::Sound->create						(sounds[eInvTradeDone],data,st_Effect,sg_SourceType);

}

void CUITradeWnd::InitTrade(CInventoryOwner* pOur, CInventoryOwner* pOthers)
{
	VERIFY								(pOur);
	VERIFY								(pOthers);

	m_pInvOwner							= pOur;
	m_pOthersInvOwner					= pOthers;
	UIOthersPriceCaption.GetPhraseByIndex(0)->SetText(*CStringTable().translate("ui_st_opponent_items"));

	UICharacterInfoLeft.InitCharacter(m_pInvOwner->object_id());
	UICharacterInfoRight.InitCharacter(m_pOthersInvOwner->object_id());

	m_pInv								= &m_pInvOwner->inventory();
	m_pOthersInv						= pOur->GetTrade()->GetPartnerInventory();
		
	m_pTrade							= pOur->GetTrade();
	m_pOthersTrade						= pOur->GetTrade()->GetPartnerTrade();
		
	EnableAll							();

	UpdateLists							(eBoth);
	UIPropertiesBox.Hide		();

}  

void CUITradeWnd::SendMessage(CUIWindow *pWnd, s16 msg, void *pData)
{
	if(pWnd == &UIPropertiesBox &&	msg==PROPERTY_CLICKED)
	{
		ProcessPropertiesBoxClicked	();
	}
	else if(pWnd == &UIToTalkButton && msg == BUTTON_CLICKED)
	{
		SwitchToTalk();
	}
	else if(pWnd == &UIPerformTradeButton && msg == BUTTON_CLICKED)
	{
		PerformTrade();
	}

	CUIWindow::SendMessage(pWnd, msg, pData);
}

void CUITradeWnd::Draw()
{
	inherited::Draw				();
	if(UIDealMsg)		UIDealMsg->Draw();

}

extern void UpdateCameraDirection(CGameObject* pTo);

void CUITradeWnd::Update()
{
	EListType et					= eNone;

	if(m_pInv->ModifyFrame()==Device.dwFrame && m_pOthersInv->ModifyFrame()==Device.dwFrame){
		et = eBoth;
	}else if(m_pInv->ModifyFrame()==Device.dwFrame){
		et = e1st;
	}else if(m_pOthersInv->ModifyFrame()==Device.dwFrame){
		et = e2nd;
	}
	if(et!=eNone)
		UpdateLists					(et);

	inherited::Update				();
	UpdateCameraDirection			(smart_cast<CGameObject*>(m_pOthersInvOwner));

	if(UIDealMsg){
		UIDealMsg->Update();
		if( !UIDealMsg->IsActual()){
			HUD().GetUI()->UIGame()->RemoveCustomStatic("not_enough_money_mine");
			HUD().GetUI()->UIGame()->RemoveCustomStatic("not_enough_money_other");
			UIDealMsg			= nullptr;
		}
	}
}

#include "UIInventoryUtilities.h"
void CUITradeWnd::Show()
{
	InventoryUtilities::SendInfoToActor("ui_trade");
	inherited::Show					(true);
	inherited::Enable				(true);

	SetCurrentItem					(nullptr);
	ResetAll						();
	UIDealMsg				= nullptr;
	PlaySnd								(eInvSndOpen);
}

void CUITradeWnd::Hide()
{
	InventoryUtilities::SendInfoToActor("ui_trade_hide");
	inherited::Show					(false);
	inherited::Enable				(false);
	if(bStarted)
	{
		StopTrade					();
		PlaySnd								(eInvSndClose);
	}
	
	UIDealMsg				= nullptr;

	if(HUD().GetUI()->UIGame()){
		HUD().GetUI()->UIGame()->RemoveCustomStatic("not_enough_money_mine");
		HUD().GetUI()->UIGame()->RemoveCustomStatic("not_enough_money_other");
	}
	UIPropertiesBox.RemoveAll();
	UIPropertiesBox.Hide();
	SetCurrentItem(nullptr);
	if (CUIDragDropListEx::m_drag_item)
		delete_data(CUIDragDropListEx::m_drag_item);
	std::for_each(sourceDragDropLists.begin(),sourceDragDropLists.end(),[](CUIDragDropListEx* list)
	{
		list->GetParent()->SetCapture(nullptr, false);
		list->ClearAll(true);
	});
}

void CUITradeWnd::StartTrade()
{
	if (m_pTrade)					m_pTrade->TradeCB(true);
	if (m_pOthersTrade)				m_pOthersTrade->TradeCB(true);
	bStarted						= true;
}

void CUITradeWnd::StopTrade()
{
	if (m_pTrade)					m_pTrade->TradeCB(false);
	if (m_pOthersTrade)				m_pOthersTrade->TradeCB(false);
	bStarted						= false;
}

#include "../trade_parameters.h"

bool CUITradeWnd::CanMoveToOther(PIItem pItem)
{
	float r1				= CalcItemsWeight(&UIOurTradeList);	// our
	float r2				= CalcItemsWeight(&UIOthersTradeList);	// other

	float itmWeight			= pItem->Weight();
	float otherInvWeight	= m_pOthersInv->CalcTotalWeight();
	float otherMaxWeight	= m_pOthersInv->GetMaxWeight();

	if (!m_pOthersInvOwner->trade_parameters().enabled(
			CTradeParameters::action_buy(0),
			pItem->object().cNameSect()
		))
		return				(false);

	if(otherInvWeight-r2+r1+itmWeight > otherMaxWeight)
		return				false;

	return true;
}

void CUITradeWnd::move_item(CUICellItem* itm, CUIDragDropListEx* from, CUIDragDropListEx* to) 
{
	CUICellItem* _itm		= from->RemoveItem	(itm, false);
	AddSingleItemToList(_itm,to);
}


void CUITradeWnd::AddSingleItemToList(CUICellItem* itm,CUIDragDropListEx* to)
{
		CInventoryItem* iitem=static_cast<CInventoryItem*>(itm->m_pData);
		if (g_uCommonFlags.is(E_COMMON_FLAGS::uiShowTradeSB) && OPFuncs::IsUsedInInventory(m_pInvOwner,iitem))
		{
			itm->SetColor				(color_rgba(255,100,100,255));
			itm->SetMoveableToOther(!!g_uCommonFlags.is(E_COMMON_FLAGS::uiAllowOpTradeSB));
			itm->SetAllowedGrouping(false);
			to->SetItem(itm);
		}
		else
		{
			itm->SetAllowedGrouping(true);
			itm->m_bIgnoreItemPlace = iitem->m_pCurrentInventory->GetOwner() != g_actor;
			to->SetItem(itm);

		}
}

bool CUITradeWnd::ToOurTrade()
{
	if (!CurrentItem()->GetMoveableToOther() || !CanMoveToOther(CurrentIItem()))	return false;

	move_item				(CurrentItem(), &UIOurBagList, &UIOurTradeList);
	UpdatePrices			();
	return					true;
}

bool CUITradeWnd::ToOthersTrade()
{
	move_item				(CurrentItem(), &UIOthersBagList, &UIOthersTradeList);
	UpdatePrices			();
	return					true;
}

bool CUITradeWnd::ToOurBag()
{
	move_item				(CurrentItem(), &UIOurTradeList, &UIOurBagList);
	UpdatePrices			();
	return					true;
}

bool CUITradeWnd::ToOthersBag()
{
	move_item				(CurrentItem(), &UIOthersTradeList, &UIOthersBagList);
	UpdatePrices			();
	return					true;
}

float CUITradeWnd::CalcItemsWeight(CUIDragDropListEx* pList)
{
	float res = 0.0f;

	for(u32 i=0; i<pList->ItemsCount(); ++i)
	{
		CUICellItem* itm	= pList->GetItemIdx	(i);
		PIItem	iitem		= static_cast<PIItem>(itm->m_pData);
		res					+= iitem->Weight();
		for(u32 j=0; j<itm->ChildsCount(); ++j){
			PIItem	jitem		= static_cast<PIItem>(itm->Child(j)->m_pData);
			res					+= jitem->Weight();
		}
	}
	return res;
}

u32 CUITradeWnd::CalcItemsPrice(CUIDragDropListEx* pList, CTrade* pTrade, bool bBuying)
{
	u32 iPrice				= 0;
	
	for(u32 i=0; i<pList->ItemsCount(); ++i)
	{
		CUICellItem* itm	= pList->GetItemIdx(i);
		PIItem iitem		= static_cast<PIItem>(itm->m_pData);
		iPrice				+= pTrade->GetItemPrice(iitem, bBuying);

		for(u32 j=0; j<itm->ChildsCount(); ++j){
			PIItem jitem	= static_cast<PIItem>(itm->Child(j)->m_pData);
			iPrice			+= pTrade->GetItemPrice(jitem, bBuying);
		}
	}

	return iPrice;
}

void CUITradeWnd::PerformTrade()
{

	if (UIOurTradeList.ItemsCount()==0 && UIOthersTradeList.ItemsCount()==0) 
		return;

	int our_money			= m_pInvOwner->get_money();
	int others_money		= m_pOthersInvOwner->get_money();

	int delta_price			= int(m_iOurTradePrice-m_iOthersTradePrice);

	our_money				+= delta_price;
	others_money			-= delta_price;

	if(our_money>=0 && others_money>=0 && (m_iOurTradePrice>=0 || m_iOthersTradePrice>0))
	{
		PlaySnd(eInvTradeDone);
		m_pOthersTrade->OnPerformTrade(m_iOthersTradePrice, m_iOurTradePrice);
		
		TransferItems		(&UIOurTradeList,		&UIOthersBagList, m_pOthersTrade,	true);
		TransferItems		(&UIOthersTradeList,	&UIOurBagList,	m_pOthersTrade,	false);
	}else
	{
		if(others_money<0)
			UIDealMsg		= HUD().GetUI()->UIGame()->AddCustomStatic("not_enough_money_other", true);
		else
			UIDealMsg		= HUD().GetUI()->UIGame()->AddCustomStatic("not_enough_money_mine", true);


		UIDealMsg->m_endTime	= Device.fTimeGlobal+2.0f;// sec
	}
	SetCurrentItem			(nullptr);
}

void CUITradeWnd::DisableAll()
{
	std::for_each(sourceDragDropLists.begin(),sourceDragDropLists.end(),[](CUIDragDropListEx* list){list->Enable(false);});
}

void CUITradeWnd::EnableAll()
{
	std::for_each(sourceDragDropLists.begin(),sourceDragDropLists.end(),[](CUIDragDropListEx* list){list->Enable(true);});
}

void CUITradeWnd::UpdatePrices()
{
	m_iOurTradePrice	= CalcItemsPrice	(&UIOurTradeList,		m_pOthersTrade, true);
	m_iOthersTradePrice = CalcItemsPrice	(&UIOthersTradeList,	m_pOthersTrade, false);


	shared_str moneyRegional=CStringTable().translate("ui_st_money_regional");
	string256				buf;
	sprintf_s					(buf, "%d %s", m_iOurTradePrice,moneyRegional.c_str());

	UIOurPriceCaption.GetPhraseByIndex(2)->str = buf;
	sprintf_s					(buf, "%d %s", m_iOthersTradePrice,moneyRegional.c_str());
	UIOthersPriceCaption.GetPhraseByIndex(2)->str = buf;

	sprintf_s					(buf, "%d %s", m_pInvOwner->get_money(),moneyRegional.c_str());
	UIOurMoneyStatic.SetText(buf);

	if(!m_pOthersInvOwner->InfinitiveMoney()){
		sprintf_s					(buf, "%d %s", m_pOthersInvOwner->get_money(),moneyRegional.c_str());
		UIOtherMoneyStatic.SetText(buf);
	}else
	{
		UIOtherMoneyStatic.SetText("---");
	}
}

void CUITradeWnd::TransferItems(CUIDragDropListEx* pSellList,CUIDragDropListEx* pBuyList,CTrade* pTrade,bool bBuying)
{
	while(pSellList->ItemsCount())
	{
		CUICellItem* itm	=	pSellList->RemoveItem(pSellList->GetItemIdx(0),false);
		pTrade->TransferItem	((PIItem)itm->m_pData, bBuying);
		pBuyList->SetItem		(itm);
	}

	pTrade->pThis.inv_owner->set_money ( pTrade->pThis.inv_owner->get_money(), true );
	pTrade->pPartner.inv_owner->set_money( pTrade->pPartner.inv_owner->get_money(), true );
}

void CUITradeWnd::UpdateLists(EListType mode)
{
	if(mode==eBoth||mode==e1st){
		UIOurBagList.ClearAll(true);
		UIOurTradeList.ClearAll(true);
	}

	if(mode==eBoth||mode==e2nd){
		UIOthersBagList.ClearAll(true);
		UIOthersTradeList.ClearAll(true);
	}

	UpdatePrices						();

	if(mode==eBoth||mode==e1st){
		ruck_list.clear					();
		bool useAdds=!!g_uCommonFlags.is(E_COMMON_FLAGS::uiShowTradeSB);
		m_pInv->AddAvailableItems		(ruck_list, true,useAdds,useAdds);
		std::sort						(ruck_list.begin(),ruck_list.end(),InventoryUtilities::GreaterRoomInRuck);
		FillList						(ruck_list, UIOurBagList, true);
	}
	if(mode==eBoth||mode==e2nd){
		ruck_list.clear					();
		m_pOthersInv->AddAvailableItems	(ruck_list, true);
		std::sort						(ruck_list.begin(),ruck_list.end(),InventoryUtilities::GreaterRoomInRuck);
		FillList						(ruck_list, UIOthersBagList, false);
	}
}

void CUITradeWnd::FillList	(TIItemContainer& cont, CUIDragDropListEx& dragDropList, bool do_colorize)
{
	TIItemContainer::iterator it	= cont.begin();
	TIItemContainer::iterator it_e	= cont.end();

	for(; it != it_e; ++it) 
	{
		CUICellItem* itm			= create_cell_item	(*it);
		if(do_colorize)				ColorizeItem		(itm, CanMoveToOther(*it));
		AddSingleItemToList(itm,&dragDropList);
	}

}

bool CUITradeWnd::OnItemStartDrag(CUICellItem* itm)
{
	return false; //default behaviour
}

bool CUITradeWnd::OnItemSelected(CUICellItem* itm)
{
	SetCurrentItem		(itm);
	return				false;
}

bool CUITradeWnd::OnItemRButtonClick(CUICellItem* itm)
{
	SetCurrentItem				(itm);
	ActivatePropertiesBox		();
	return						false;
}

bool CUITradeWnd::OnItemFocusLost(CUICellItem* itm)
{
	if (itm)
		itm->SetFocused(false);
	return						false;
}

bool CUITradeWnd::OnItemFocusReceive(CUICellItem* itm)
{
	if (itm)
		itm->SetFocused(true);
	return						false;
}

bool CUITradeWnd::OnMouse(float x, float y, EUIMessages mouse_action)
{
	if(mouse_action == WINDOW_RBUTTON_DOWN)
	{
		if(UIPropertiesBox.IsShown())
		{
			UIPropertiesBox.Hide		();
			return						true;
		}
	}
	CUIWindow::OnMouse					(x, y, mouse_action);
	return true; 
}

void CUITradeWnd::SendEvent_Item_Drop(PIItem pItem)
{
	pItem->SetDropManual			(TRUE);

	if( OnClient() )
	{
		NET_Packet					P;
		pItem->object().u_EventGen	(P, GE_OWNERSHIP_REJECT, pItem->object().H_Parent()->ID());
		P.w_u16						(pItem->object().ID());
		pItem->object().u_EventSend(P);
	}
	PlaySnd				(eInvDropItem);
}

bool CUITradeWnd::OnItemDrop(CUICellItem* itm)
{
	CUIDragDropListEx*	old_owner		= itm->OwnerList();
	CUIDragDropListEx*	new_owner		= CUIDragDropListEx::m_drag_item->BackList();
	if(old_owner==new_owner || !old_owner || !new_owner)
					return false;

	if(old_owner==&UIOurBagList && new_owner==&UIOurTradeList)
		ToOurTrade				();
	else if(old_owner==&UIOurTradeList && new_owner==&UIOurBagList)
		ToOurBag				();
	else if(old_owner==&UIOthersBagList && new_owner==&UIOthersTradeList)
		ToOthersTrade			();
	else if(old_owner==&UIOthersTradeList && new_owner==&UIOthersBagList)
		ToOthersBag				();
	else if (old_owner==new_owner && new_owner==&UIOurTradeList)
		Msg("drop item not implement.");
	return true;
}

bool CUITradeWnd::OnItemDbClick(CUICellItem* itm)
{
	SetCurrentItem						(itm);
	CUIDragDropListEx*	old_owner		= itm->OwnerList();
	
	if(old_owner == &UIOurBagList)
		ToOurTrade				();
	else if(old_owner == &UIOurTradeList)
		ToOurBag				();
	else if(old_owner == &UIOthersBagList)
		ToOthersTrade			();
	else if(old_owner == &UIOthersTradeList)
		ToOthersBag				();
	else
		R_ASSERT2(false, "wrong parent for cell item");
	UpdateItemUICost(itm);
	return true;
}


CUICellItem* CUITradeWnd::CurrentItem()
{
	return m_pCurrentCellItem;
}

PIItem CUITradeWnd::CurrentIItem()
{
	return	(m_pCurrentCellItem)?static_cast<PIItem>(m_pCurrentCellItem->m_pData) : nullptr;
}

void CUITradeWnd::UpdateItemUICost(CUICellItem* cellItem)
{
	if(cellItem && UIItemInfo.UICost)
	{
		CUIDragDropListEx* owner	= cellItem->OwnerList();
		bool bBuying				= (owner==&UIOurBagList) || (owner==&UIOurTradeList);
		string64			str;
		u32 singleItem=m_pOthersTrade->GetItemPrice(CurrentIItem(), bBuying);
		u32 totalCost=0;
		if (cellItem->HasChilds())
			for(size_t i=0; i<cellItem->ChildsCount(); ++i)
			{
				CInventoryItem* item=cellItem->Child(i)->m_pData ? static_cast<CInventoryItem*>(cellItem->Child(i)->m_pData): nullptr;
				if (item)
					totalCost+=m_pOthersTrade->GetItemPrice(item, bBuying);
			}
		if (totalCost>0)
			sprintf_s				(str, "%d/%d %s", singleItem,totalCost+singleItem,*CStringTable().translate("ui_st_money_regional"));
		else
			sprintf_s				(str, "%d %s", singleItem,*CStringTable().translate("ui_st_money_regional"));
		UIItemInfo.UICost->SetText (str);
	}
}

void CUITradeWnd::SetCurrentItem(CUICellItem* itm)
{
	if(m_pCurrentCellItem == itm) return;
	SetItemSelected(itm);
	m_pCurrentCellItem				= itm;
	UIItemInfo.InitItem	(CurrentIItem());
	ClearAllSuitables();
	if(!m_pCurrentCellItem)		return;

	UpdateItemUICost(itm);

	bool processed=false;
	auto currentIItem=CurrentIItem();
	std::for_each(sourceDragDropLists.begin(),sourceDragDropLists.end(),[&processed,currentIItem](CUIDragDropListEx* list)
	{
		bool ls=list->select_suitables_by_item(currentIItem);
		processed=processed || ls;
	});
	if (pSettings->line_exist("maingame_ui", "on_cell_after_select"))
	{
		CGameObject* GO = smart_cast<CGameObject*>(CurrentIItem());
		if (GO)
		{
			LPCSTR on_cell_after_select = pSettings->r_string("maingame_ui", "on_cell_after_select");
			luabind::functor<void> functor;
			if (!ai().script_engine().functor(on_cell_after_select, functor))
			{
				Msg("! ERROR function [%s] not exist for on_cell_after_select callback", on_cell_after_select);
			}
			else
			{
				try
				{
					functor(this, CurrentItem(), processed);
				}
				catch (...)
				{
					Msg("! ERROR function [%s] cause unknown error.", on_cell_after_select);
				}
			}
		}
	}
}

void CUITradeWnd::SetItemSelected(CUICellItem* itm)
{
	CUICellItem* curr=CurrentItem();
	if (curr!=nullptr  && curr->GetSelected())
		curr->SetSelected(false);
	if (itm!=nullptr && !itm->GetSelected())	
		itm->SetSelected(true);
}

void CUITradeWnd::SwitchToTalk()
{
	GetMessageTarget()->SendMessage		(this, TRADE_WND_CLOSED);
}

void CUITradeWnd::BindDragDropListEvents(CUIDragDropListEx* lst)
{
	lst->m_f_item_drop				= CUIDragDropListEx::DRAG_DROP_EVENT(this,&CUITradeWnd::OnItemDrop);
	lst->m_f_item_start_drag		= CUIDragDropListEx::DRAG_DROP_EVENT(this,&CUITradeWnd::OnItemStartDrag);
	lst->m_f_item_db_click			= CUIDragDropListEx::DRAG_DROP_EVENT(this,&CUITradeWnd::OnItemDbClick);
	lst->m_f_item_selected			= CUIDragDropListEx::DRAG_DROP_EVENT(this,&CUITradeWnd::OnItemSelected);
	lst->m_f_item_rbutton_click		= CUIDragDropListEx::DRAG_DROP_EVENT(this,&CUITradeWnd::OnItemRButtonClick);
	lst->m_f_item_focus_lost		= CUIDragDropListEx::DRAG_DROP_EVENT(this,&CUITradeWnd::OnItemFocusLost);
	lst->m_f_item_focus_received	= CUIDragDropListEx::DRAG_DROP_EVENT(this,&CUITradeWnd::OnItemFocusReceive);
}

void CUITradeWnd::PlaySnd(eTradeSoundActions a)
{
	if (sounds[a]._handle())
		sounds[a].play(nullptr, sm_2D);
}



void CUITradeWnd::ColorizeItem(CUICellItem* itm, bool b)
{
	if(!b)
		itm->SetColor				(color_rgba(255,100,100,255));
}

void CUITradeWnd::DetachAddon(const char* addon_name)
{
	PlaySnd										(eInvDetachAddon);
	OPFuncs::DetachAddon(CurrentIItem(),addon_name);
	SetCurrentItem(nullptr);
}

void CUITradeWnd::ProcessPropertiesBoxClicked	()
{
	CUIListBoxItem* clickedItem=UIPropertiesBox.GetClickedItem();
	if(clickedItem)
	{
		u32 itemTag=clickedItem->GetTAG();
		switch(itemTag)
		{
		case INVENTORY_DISCHARGE_EXO:
			if (CExoOutfit* exo = (CExoOutfit*)clickedItem->GetData())
				exo->RemoveFromBatterySlot(true);
			break;
		case INVENTORY_DETACH_GRENADE_LAUNCHER_ADDON:
			DetachAddon(*(smart_cast<CWeapon*>(CurrentIItem()))->GetGrenadeLauncherName());
			break;
		case INVENTORY_DETACH_SCOPE_ADDON:
			DetachAddon(*(smart_cast<CWeapon*>(CurrentIItem()))->GetScopeName());
			break;
		case INVENTORY_DETACH_SILENCER_ADDON:
			DetachAddon(*(smart_cast<CWeapon*>(CurrentIItem()))->GetSilencerName());
			break;

		case INVENTORY_UNLOAD_MAGAZINE:
			{
				CUICellItem * itm = CurrentItem();
				CWeapon* weapon=static_cast<CWeapon*>(itm->m_pData);
				if (!weapon)
					break;
				CWeaponMagazined* wg = smart_cast<CWeaponMagazined*>(weapon);
				if (!wg)
					break;
				wg->PlayEmptySnd();
				OPFuncs::UnloadWeapon(wg);
				for(size_t i=0; i<itm->ChildsCount(); ++i)
				{
					CUICellItem * child_itm			= itm->Child(i);
					OPFuncs::UnloadWeapon(smart_cast<CWeaponMagazined*>(static_cast<CWeapon*>(child_itm->m_pData)));
				}
				SetCurrentItem(nullptr);
			}
			break;
		case INVENTORY_CB_MOVE_ALL:
		case INVENTORY_CB_MOVE_SINGLE:
			{
				CActor *pActor				= smart_cast<CActor*>(Level().CurrentEntity());
				CUICellItem* moveItem=static_cast<CUICellItem*>(clickedItem->GetData());
				if(!pActor && !moveItem)
					break;
				if (itemTag==INVENTORY_CB_MOVE_ALL)
				{
					while (moveItem->ChildsCount()!=0)
					{
						CUICellItem* cellItem = moveItem->Child(0);
						OnItemDbClick(cellItem);
					}
				}
				OnItemDbClick(moveItem);
			}
			break;
		case INVENTORY_DROP_ACTION:
			{
				void* d = clickedItem->GetData();
				bool b_all = (d== reinterpret_cast<void*>(33));

				CActor *pActor			= smart_cast<CActor*>(Level().CurrentEntity());
				if(!pActor)				return;
				if(!b_all && CurrentIItem() && !CurrentIItem()->IsQuestItem())
				{
					SendEvent_Item_Drop		(CurrentIItem());
					SetCurrentItem			(nullptr);
					//InventoryUtilities::UpdateWeight			(UIBagWnd, true);
					break;
				}
				if(b_all && CurrentIItem() && !CurrentIItem()->IsQuestItem())
				{
						u32 cnt = CurrentItem()->ChildsCount();
						for(u32 i=0; i<cnt; ++i){
							CUICellItem*	itm				= CurrentItem()->PopChild();
							PIItem			iitm			= static_cast<PIItem>(itm->m_pData);
							SendEvent_Item_Drop				(iitm);
					}
					SendEvent_Item_Drop					(CurrentIItem());
					SetCurrentItem						(nullptr);
					//InventoryUtilities::UpdateWeight	(UIBagWnd, true);
					break;
				}
			}
			break;
		case INVENTORY_EAT_ACTION:
			{
				CInventoryItem* itm=CurrentIItem();
				SetCurrentItem							(nullptr);
				if(!itm->Useful())						return;
				CGBox*			pBox = smart_cast<CGBox*>			(itm);
				NET_Packet						P;
				if (pBox)
					itm->object().u_EventGen(P, GEG_PLAYER_ITEM_USE, itm->object().H_Parent()->ID());
				else
					itm->object().u_EventGen(P, GEG_PLAYER_ITEM_EAT, itm->object().H_Parent()->ID());
				P.w_u16(itm->object().ID());
				itm->object().u_EventSend(P);
				PlaySnd									(eInvItemUse);
			}
			break;
		default:break;
		}
		UpdateItemUICost(CurrentItem());
	}
}

void CUITradeWnd::ActivatePropertiesBox()
{
	IWListTypes wtype = CurrentItem()->OwnerList()->GetUIListId();
	bool oppLists= wtype ==IWListTypes::ltTradeOurTrade || wtype == IWListTypes::ltTradeOurBag;
	UIPropertiesBox.RemoveAll();

	bool b_show=false;
	if (oppLists)
	{
		CWeapon*			pWeapon				= smart_cast<CWeapon*>			(CurrentIItem());
		CMedkit*			pMedkit				= smart_cast<CMedkit*>			(CurrentIItem());
		CAntirad*			pAntirad			= smart_cast<CAntirad*>			(CurrentIItem());
		CEatableItem*		pEatableItem		= smart_cast<CEatableItem*>		(CurrentIItem());
		CBottleItem*		pBottleItem			= smart_cast<CBottleItem*>		(CurrentIItem());
		CGBox*			pBox = smart_cast<CGBox*>			(CurrentIItem());
		CExoOutfit*			pExo = smart_cast<CExoOutfit*>		(CurrentIItem());
		if (pWeapon)
		{
			if(pWeapon->GrenadeLauncherAttachable() && pWeapon->IsGrenadeLauncherAttached())
			{
				UIPropertiesBox.AddItem(OPFuncs::getComplexString("st_detach_gl",nullptr, OPFuncs::getAddonInvName(pWeapon->GetGrenadeLauncherName().c_str()),"st_detach_gl_full").c_str(),  nullptr, INVENTORY_DETACH_GRENADE_LAUNCHER_ADDON);
				b_show			= true;
			}
			if(pWeapon->ScopeAttachable() && pWeapon->IsScopeAttached())
			{
				UIPropertiesBox.AddItem(OPFuncs::getComplexString("st_detach_scope",nullptr, OPFuncs::getAddonInvName(pWeapon->GetScopeName().c_str()),"st_detach_scope_full").c_str(),  nullptr, INVENTORY_DETACH_SCOPE_ADDON);
				b_show			= true;
			}
			if(pWeapon->SilencerAttachable() && pWeapon->IsSilencerAttached())
			{
				UIPropertiesBox.AddItem(OPFuncs::getComplexString("st_detach_silencer",nullptr, OPFuncs::getAddonInvName(pWeapon->GetSilencerName().c_str()),"st_detach_silencer_full").c_str(),  nullptr, INVENTORY_DETACH_SILENCER_ADDON);
				b_show			= true;
			}
			if(smart_cast<CWeaponMagazined*>(pWeapon) && IsGameTypeSingle())
			{
				bool b = (0!=pWeapon->GetAmmoElapsed());
				if(!b)
				{
					CUICellItem * itm = CurrentItem();
					for(size_t i=0; i<itm->ChildsCount(); ++i)
					{
						pWeapon		= smart_cast<CWeaponMagazined*>(static_cast<CWeapon*>(itm->Child(i)->m_pData));
						if(pWeapon->GetAmmoElapsed())
						{
							b = true;
							break;
						}
					}
				}
				if(b){
					UIPropertiesBox.AddItem(OPFuncs::getComplexString("st_unload_magazine",pWeapon).c_str(),  nullptr, INVENTORY_UNLOAD_MAGAZINE);
					b_show			= true;
				}
			}
		}
		else if (pExo)
		{
			if (pExo->isBatteryPresent())
			{
				UIPropertiesBox.AddItem("st_discharge_exo", pExo, INVENTORY_DISCHARGE_EXO);
				b_show = true;
			}
		}
		LPCSTR _action = nullptr;
		if(pMedkit || pAntirad || pBox)
			_action					= "st_use";
		else if(pEatableItem)
			if(pBottleItem)
				_action					= "st_drink";
			else
				_action					= "st_eat";

		if(_action){
			UIPropertiesBox.AddItem(_action,  nullptr, INVENTORY_EAT_ACTION);
			b_show			= true;
		}

		if(!CurrentIItem()->IsQuestItem())
		{
			if (CurrentItem()->GetMoveableToOther() && CanMoveToOther(CurrentIItem()))
			{
				UIPropertiesBox.AddItem("ui_carbody_move_single", CurrentItem(), INVENTORY_CB_MOVE_SINGLE);
				if (CurrentItem()->ChildsCount())
					UIPropertiesBox.AddItem("ui_carbody_move_all", CurrentItem(), INVENTORY_CB_MOVE_ALL);
				b_show			= true;
			}
			//commented due to use this for hack in some quests...
			/*if (CurrentItem()->OwnerList()==&UIOurBagList)
			{
				UIPropertiesBox.AddItem("st_drop", nullptr, INVENTORY_DROP_ACTION);
				if(CurrentItem()->ChildsCount())
					UIPropertiesBox.AddItem("st_drop_all", reinterpret_cast<void*>(33), INVENTORY_DROP_ACTION);
				b_show			= true;
			}*/
		}
	}
	else
	{
		if (CurrentItem()->GetMoveableToOther() && CanMoveToOther(CurrentIItem()))
		{
			UIPropertiesBox.AddItem("ui_carbody_move_single", CurrentItem(), INVENTORY_CB_MOVE_SINGLE);
			if (CurrentItem()->ChildsCount())
				UIPropertiesBox.AddItem("ui_carbody_move_all", CurrentItem(), INVENTORY_CB_MOVE_ALL);
			b_show			= true;
		}
	}

	if (b_show)
	{
		UIPropertiesBox.AutoUpdateSize	();
		UIPropertiesBox.BringAllToTop	();
		Fvector2						cursor_pos2;
		Frect							vis_rect;
		GetAbsoluteRect					(vis_rect);
		cursor_pos2						= GetUICursor()->GetCursorPosition();
		cursor_pos2.sub					(vis_rect.lt);
		UIPropertiesBox.Show			(vis_rect, cursor_pos2);
		PlaySnd							(eInvProperties);
		SetCapture(static_cast<CUIWindow*>(&UIPropertiesBox),true);
	}	
}