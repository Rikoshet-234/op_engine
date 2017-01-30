#include "stdafx.h"
#include "../../defines.h"
#include "UIInventoryWnd.h"
#include "../actor.h"
#include "../silencer.h"
#include "../scope.h"
#include "../grenadelauncher.h"
#include "../Artifact.h"
#include "../eatable_item.h"
#include "../BottleItem.h"
#include "../WeaponMagazined.h"
#include "../inventory.h"
#include "../game_base.h"
#include "../game_cl_base.h"
#include "../xr_level_controller.h"
#include "UICellItem.h"
#include "../UIGameSP.h"
#include "UIListBoxItem.h"
#include "../CustomOutfit.h"
#include "../string_table.h"
#include "../WeaponMagazinedWGrenade.h"
#include "../game_object_space.h"
#include "../script_callback_ex.h"
#include "../script_game_object.h"
#include "../OPFuncs/utils.h"



void CUIInventoryWnd::EatItem(PIItem itm)
{
	SetCurrentItem							(nullptr);
	if(!itm->Useful())						return;

	SendEvent_Item_Eat						(itm);

	PlaySnd									(eInvItemUse);
}

#include "../Medkit.h"
#include "../Antirad.h"

void CUIInventoryWnd::AddWeaponAttachInfo(u32 slot,CInventoryItem* addon,std::string titleId,bool& needShow)
{
	if(m_pInv->m_slots[slot].m_pIItem != NULL && m_pInv->m_slots[slot].m_pIItem->CanAttach(addon))
	{
		PIItem target = m_pInv->m_slots[slot].m_pIItem;
		string512 buf;
		LPCSTR name;
		LPCSTR between=CStringTable().translate("st_attach_between").c_str();
		if (g_uCommonFlags.test(E_COMMON_FLAGS::uiShowExtDesc))
			name=target->Name();
		else
			if (xr_strlen(target->NameShort())>0)
				name=target->NameShort();
			else
				name="no_short_name";
		sprintf_s(buf,"%s %s %s",OPFuncs::getComplexString(titleId,addon).c_str(),between,name);
		UIPropertiesBox.AddItem(buf,  static_cast<void*>(target), INVENTORY_ATTACH_ADDON);
		if (!needShow)
			needShow = true;
	}
}

void CUIInventoryWnd::AddLoadAmmoWeaponInfo(u32 slot,CWeaponAmmo* ammo,bool& needShow)
{
	auto slotWeapon=m_pInv->m_slots[slot].m_pIItem;
	if (slotWeapon != NULL && slotWeapon->CanLoadAmmo(ammo,true))
	{
		CWeaponMagazined* weapon=smart_cast<CWeaponMagazined*>(slotWeapon);
		UIPropertiesBox.AddItem(OPFuncs::getComplexString("st_load_ammo",weapon).c_str(),  weapon, INVENTORY_LOAD_MAGAZINE);
		if (!needShow)
			needShow=true;
	}
}

void CUIInventoryWnd::ActivatePropertiesBox()
{
	// Флаг-признак для невлючения пункта контекстного меню: Dreess Outfit, если костюм уже надет
	bool bAlreadyDressed = false; 

		
	UIPropertiesBox.RemoveAll();

	CMedkit*			pMedkit				= smart_cast<CMedkit*>			(CurrentIItem());
	CAntirad*			pAntirad			= smart_cast<CAntirad*>			(CurrentIItem());
	CEatableItem*		pEatableItem		= smart_cast<CEatableItem*>		(CurrentIItem());
	CCustomOutfit*		pOutfit				= smart_cast<CCustomOutfit*>	(CurrentIItem());
	CWeapon*			pWeapon				= smart_cast<CWeapon*>			(CurrentIItem());
	CScope*				pScope				= smart_cast<CScope*>			(CurrentIItem());
	CSilencer*			pSilencer			= smart_cast<CSilencer*>		(CurrentIItem());
	CGrenadeLauncher*	pGrenadeLauncher	= smart_cast<CGrenadeLauncher*>	(CurrentIItem());
	CBottleItem*		pBottleItem			= smart_cast<CBottleItem*>		(CurrentIItem());
#pragma region disable due to hard learning async state machine and plaing animation
	//CWeaponAmmo*		pAmmoItem			= smart_cast<CWeaponAmmo*>		(CurrentIItem());
#pragma endregion
	
	bool	b_show			= false;
	bool	b_picked=false;

	if(!pOutfit && CurrentIItem()->GetSlot()!=NO_ACTIVE_SLOT && !m_pInv->m_slots[CurrentIItem()->GetSlot()].m_bPersistent && m_pInv->CanPutInSlot(CurrentIItem()) )
	{
		UIPropertiesBox.AddItem(OPFuncs::getComplexString("st_move_to_slot",CurrentIItem()).c_str(),  nullptr, INVENTORY_TO_SLOT_ACTION);
		b_show			= b_picked = true;
	}
	if(CurrentIItem()->Belt() && m_pInv->CanPutInBelt(CurrentIItem()))
	{
		UIPropertiesBox.AddItem(OPFuncs::getComplexString("st_move_on_belt",CurrentIItem()).c_str(),  nullptr, INVENTORY_TO_BELT_ACTION);
		b_show			= true;
	}

	if(CurrentIItem()->Ruck() && m_pInv->CanPutInRuck(CurrentIItem()) && (CurrentIItem()->GetSlot()==size_t(-1) || !m_pInv->m_slots[CurrentIItem()->GetSlot()].m_bPersistent) )
	{
		if(!pOutfit)
			UIPropertiesBox.AddItem(OPFuncs::getComplexString("st_move_to_bag",CurrentIItem()).c_str(),  nullptr, INVENTORY_TO_BAG_ACTION);
		else
			UIPropertiesBox.AddItem(OPFuncs::getComplexString("st_undress_outfit",pOutfit).c_str(),  nullptr, INVENTORY_TO_BAG_ACTION);
		bAlreadyDressed = true;
		b_show			= true;
	}
	if(pOutfit  && !bAlreadyDressed )
	{
		UIPropertiesBox.AddItem(OPFuncs::getComplexString("st_dress_outfit",pOutfit).c_str(),  nullptr, INVENTORY_TO_SLOT_ACTION);
		b_show			= true;
	}
	
	//отсоединение аддонов от вещи
	if(pWeapon)
	{
		//оружие можно засунуть в слот, даже если там не пусто
		if (!b_picked 
			&& CurrentItem()->OwnerList()!=m_pUIAutomaticList 
			&& CurrentItem()->OwnerList()!=m_pUIPistolList
			&& CurrentItem()->OwnerList()!=m_pUIKnifeList
			&& CurrentItem()->OwnerList()!=m_pUIShotgunList
			&& CurrentItem()->OwnerList()!=m_pUIApparatusList) //не надо издеваться над CanPutInSlot , ему и так по жизни трудно
			UIPropertiesBox.AddItem(OPFuncs::getComplexString("st_move_to_slot",CurrentIItem()).c_str(),  nullptr, INVENTORY_TO_SLOT_ACTION);

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
	
	//присоединение аддонов к активному слоту 
	if(pScope)
	{
		AddWeaponAttachInfo(PISTOL_SLOT,pScope,"st_attach_scope_to_pistol",b_show);
		AddWeaponAttachInfo(SHOTGUN_SLOT,pScope,"st_attach_scope_to_shotgun",b_show);
		AddWeaponAttachInfo(RIFLE_SLOT,pScope,"st_attach_scope_to_rifle",b_show);
	}
	else if(pSilencer)
	{
		AddWeaponAttachInfo(PISTOL_SLOT,pSilencer,"st_attach_silencer_to_pistol",b_show);
		AddWeaponAttachInfo(SHOTGUN_SLOT,pSilencer,"st_attach_silencer_to_shotgun",b_show);
		AddWeaponAttachInfo(RIFLE_SLOT,pSilencer,"st_attach_silencer_to_rifle",b_show);
	}
	else if(pGrenadeLauncher)
	{
		AddWeaponAttachInfo(PISTOL_SLOT,pGrenadeLauncher,"st_attach_gl_to_rifle",b_show);
		AddWeaponAttachInfo(SHOTGUN_SLOT,pGrenadeLauncher,"st_attach_gl_to_shotgun",b_show);
		AddWeaponAttachInfo(RIFLE_SLOT,pGrenadeLauncher,"st_attach_gl_to_rifle",b_show);
	}

#pragma region disable due to hard learning async state machine and plaing animation
	/*if (pAmmoItem)
	{
		AddLoadAmmoWeaponInfo(PISTOL_SLOT,pAmmoItem,b_show);
		AddLoadAmmoWeaponInfo(SHOTGUN_SLOT,pAmmoItem,b_show);
		AddLoadAmmoWeaponInfo(RIFLE_SLOT,pAmmoItem,b_show);
	}*/
#pragma endregion
	LPCSTR _action = nullptr;

	if(pMedkit || pAntirad)
	{
		_action					= "st_use";
	}
	else if(pEatableItem)
	{
		if(pBottleItem)
			_action					= "st_drink";
		else
			_action					= "st_eat";
	} 
	

	if(_action){
		UIPropertiesBox.AddItem(_action,  nullptr, INVENTORY_EAT_ACTION);
		b_show			= true;
	}

	bool disallow_drop	= (pOutfit&&bAlreadyDressed);
	disallow_drop		|= !!CurrentIItem()->IsQuestItem();

	if(!disallow_drop)
	{

		UIPropertiesBox.AddItem("st_drop", nullptr, INVENTORY_DROP_ACTION);
		b_show			= true;

		if(CurrentItem()->ChildsCount())
			UIPropertiesBox.AddItem("st_drop_all", reinterpret_cast<void*>(33), INVENTORY_DROP_ACTION);
	}

	/*CGameObject* GO = smart_cast<CGameObject*>(CurrentIItem()); 
	if (GO)
		Actor()->callback(GameObject::eOnInventoryShowPropBox)(&UIPropertiesBox,GO->lua_game_object());*/

	if(b_show)
	{
		UIPropertiesBox.AutoUpdateSize	();
		UIPropertiesBox.BringAllToTop	();
		Fvector2						cursor_pos;
		Frect							vis_rect;
		GetAbsoluteRect					(vis_rect);
		cursor_pos						= GetUICursor()->GetCursorPosition();
		cursor_pos.sub					(vis_rect.lt);
		UIPropertiesBox.Show			(vis_rect, cursor_pos);
		PlaySnd							(eInvProperties);
	}
}



void CUIInventoryWnd::ProcessPropertiesBoxClicked	()
{
	if(UIPropertiesBox.GetClickedItem())
	{
		switch(UIPropertiesBox.GetClickedItem()->GetTAG())
		{
		case INVENTORY_TO_SLOT_ACTION:	
			ToSlot(CurrentItem(), true);
			break;
		case INVENTORY_TO_BELT_ACTION:	
			ToBelt(CurrentItem(),false);
			break;
		case INVENTORY_TO_BAG_ACTION:	
			ToBag(CurrentItem(),false);
			break;
		case INVENTORY_DROP_ACTION:
			{
				void* d = UIPropertiesBox.GetClickedItem()->GetData();
				bool b_all = (d==(void*)33);

				DropCurrentItem(b_all);
			}break;
		case INVENTORY_EAT_ACTION:
			EatItem(CurrentIItem());
			break;
		case INVENTORY_ATTACH_ADDON:
			AttachAddon((PIItem)(UIPropertiesBox.GetClickedItem()->GetData()));
			break;
		case INVENTORY_DETACH_SCOPE_ADDON:
			DetachAddon(*(smart_cast<CWeapon*>(CurrentIItem()))->GetScopeName());
			break;
		case INVENTORY_DETACH_SILENCER_ADDON:
			DetachAddon(*(smart_cast<CWeapon*>(CurrentIItem()))->GetSilencerName());
			break;
		case INVENTORY_DETACH_GRENADE_LAUNCHER_ADDON:
			DetachAddon(*(smart_cast<CWeapon*>(CurrentIItem()))->GetGrenadeLauncherName());
			break;
		case INVENTORY_RELOAD_MAGAZINE:
			(smart_cast<CWeapon*>(CurrentIItem()))->Action(kWPN_RELOAD, CMD_START);
			break;
		case INVENTORY_LOAD_MAGAZINE:
			{
				CWeaponMagazined*	weapon	=static_cast<CWeaponMagazined*>(UIPropertiesBox.GetClickedItem()->GetData());
				CWeaponAmmo*		ammo	=static_cast<CWeaponAmmo*>(CurrentItem()->m_pData);
				if (weapon && ammo)
				{
					CUIGameSP* pGameSP = smart_cast<CUIGameSP*>(HUD().GetUI()->UIGame());
					pGameSP->InventoryMenu->GetHolder()->StartStopMenu(pGameSP->InventoryMenu,true);
					if (m_pInv->ActiveItem()!=weapon)
						m_pInv->ProcessSlotAction(true,weapon->GetSlot());
					weapon->LoadAmmo(ammo);
				}
			}
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

			}break;
		case INVENTORY_PROP_CALL_FUNC:
			{
				/*luabind::functor<void> &func=*static_cast<luabind::functor<void> *>(UIPropertiesBox.GetClickedItem()->GetData());
				if (func && func.is_valid())
					func();*/
			}
			break;
		}
	}
}



bool CUIInventoryWnd::TryUseItem(PIItem itm)
{
	CBottleItem*		pBottleItem			= smart_cast<CBottleItem*>		(itm);
	CMedkit*			pMedkit				= smart_cast<CMedkit*>			(itm);
	CAntirad*			pAntirad			= smart_cast<CAntirad*>			(itm);
	CEatableItem*		pEatableItem		= smart_cast<CEatableItem*>		(itm);

	if(pMedkit || pAntirad || pEatableItem || pBottleItem)
	{
		EatItem(itm);
		return true;
	}
	return false;
}

bool CUIInventoryWnd::DropItem(PIItem itm, CUIDragDropListEx* lst)
{
	if(lst==m_pUIOutfitList)
	{
		return TryUseItem			(itm);
/*
		CCustomOutfit*		pOutfit		= smart_cast<CCustomOutfit*>	(CurrentIItem());
		if(pOutfit)
			ToSlot			(CurrentItem(), true);
		else
			EatItem				(CurrentIItem());

		return				true;
*/
	}
	CUICellItem*	_citem	= lst->ItemsCount() ? lst->GetItemIdx(0) : NULL;
	PIItem _iitem	= _citem ? (PIItem)_citem->m_pData : NULL;

	if(!_iitem)						return	false;
	if(!_iitem->CanAttach(itm))		return	false;
	AttachAddon						(_iitem);

	return							true;
}
