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
	CWeaponAmmo*		pAmmoItem			= smart_cast<CWeaponAmmo*>		(CurrentIItem());
	
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
	
	//присоединение аддонов к активному слоту (2 или 3)
	if(pScope)
	{
		if(m_pInv->m_slots[PISTOL_SLOT].m_pIItem != NULL &&
		   m_pInv->m_slots[PISTOL_SLOT].m_pIItem->CanAttach(pScope))
		 {
			PIItem tgt = m_pInv->m_slots[PISTOL_SLOT].m_pIItem;
			UIPropertiesBox.AddItem(OPFuncs::getComplexString("st_attach_scope_to_pistol",pScope).c_str(),  static_cast<void*>(tgt), INVENTORY_ATTACH_ADDON);
			b_show			= true;
		 }
		 if(m_pInv->m_slots[RIFLE_SLOT].m_pIItem != NULL &&
			m_pInv->m_slots[RIFLE_SLOT].m_pIItem->CanAttach(pScope))
		 {
			PIItem tgt = m_pInv->m_slots[RIFLE_SLOT].m_pIItem;
			UIPropertiesBox.AddItem(OPFuncs::getComplexString("st_attach_scope_to_rifle",pScope).c_str(),  static_cast<void*>(tgt), INVENTORY_ATTACH_ADDON);
			b_show			= true;
		 }
	}
	else if(pSilencer)
	{
		 if(m_pInv->m_slots[PISTOL_SLOT].m_pIItem != NULL &&
		   m_pInv->m_slots[PISTOL_SLOT].m_pIItem->CanAttach(pSilencer))
		 {
			PIItem tgt = m_pInv->m_slots[PISTOL_SLOT].m_pIItem;
			UIPropertiesBox.AddItem(OPFuncs::getComplexString("st_attach_silencer_to_pistol",pSilencer).c_str(),  static_cast<void*>(tgt), INVENTORY_ATTACH_ADDON);
			b_show			= true;
		 }
		 if(m_pInv->m_slots[RIFLE_SLOT].m_pIItem != NULL &&
			m_pInv->m_slots[RIFLE_SLOT].m_pIItem->CanAttach(pSilencer))
		 {
			PIItem tgt = m_pInv->m_slots[RIFLE_SLOT].m_pIItem;
			UIPropertiesBox.AddItem(OPFuncs::getComplexString("st_attach_silencer_to_rifle",pSilencer).c_str(),  static_cast<void*>(tgt), INVENTORY_ATTACH_ADDON);
			b_show			= true;
		 }
	}
	else if(pGrenadeLauncher)
	{
		 if(m_pInv->m_slots[RIFLE_SLOT].m_pIItem != NULL &&
			m_pInv->m_slots[RIFLE_SLOT].m_pIItem->CanAttach(pGrenadeLauncher))
		 {
			PIItem tgt = m_pInv->m_slots[RIFLE_SLOT].m_pIItem;
			UIPropertiesBox.AddItem(OPFuncs::getComplexString("st_attach_gl_to_rifle",pGrenadeLauncher).c_str(),  static_cast<void*>(tgt), INVENTORY_ATTACH_ADDON);
			b_show			= true;
		 }

	}

	if (pAmmoItem)
	{
		auto pistol=m_pInv->m_slots[PISTOL_SLOT].m_pIItem;
		auto rifle=m_pInv->m_slots[RIFLE_SLOT].m_pIItem;
		if (pistol != NULL && pistol->CanLoadAmmo(pAmmoItem))
		{
			CWeaponMagazined* weapon=smart_cast<CWeaponMagazined*>(pistol);
			UIPropertiesBox.AddItem(OPFuncs::getComplexString("st_load_ammo",weapon).c_str(),  weapon, INVENTORY_LOAD_MAGAZINE);
		}
		if (rifle != NULL && rifle->CanLoadAmmo(pAmmoItem))
		{
			CWeaponMagazined* weapon=smart_cast<CWeaponMagazined*>(rifle);
			UIPropertiesBox.AddItem(OPFuncs::getComplexString("st_load_ammo",weapon).c_str(),  weapon, INVENTORY_LOAD_MAGAZINE);
		}
	}

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
						weapon->LoadAmmo(ammo);
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
