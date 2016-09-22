#include "stdafx.h"
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



void CUIInventoryWnd::EatItem(PIItem itm)
{
	SetCurrentItem							(NULL);
	if(!itm->Useful())						return;

	SendEvent_Item_Eat						(itm);

	PlaySnd									(eInvItemUse);
}

#include "../Medkit.h"
#include "../Antirad.h"

#define EMPTY_DESC "UNUSED" //omg wtf

std::string getComplexString(std::string untranslatedString,PIItem item,std::string untranslatedString2="")
{
	std::string translateString=*CStringTable().translate(untranslatedString.c_str());
	if (psActorFlags.test(AF_INV_SHOW_EXT_DESC))
	{
		std::string additionString;
		if (item!=nullptr)
		{
			additionString=item->m_name.c_str();	
			if ((item->m_nameShort!=nullptr) && (item->m_nameShort!="") && (item->m_nameShort!=EMPTY_DESC))
				additionString=item->m_nameShort.c_str();
		}
		else
			additionString=*CStringTable().translate(untranslatedString2.c_str());
		return translateString+" "+additionString;
	}
	else
		return translateString;
}

std::string getAddonInvName(std::string addonName)
{
	std::string name;
	if (addonName.size()>0)
	{
		if (pSettings->line_exist(addonName.c_str(),"inv_name_short"))
			name=pSettings->r_string(addonName.c_str(),"inv_name_short");
		if ((name=="") || (name==EMPTY_DESC))
			name=pSettings->r_string(addonName.c_str(),"inv_name");
	}
	return name;
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
	
	bool	b_show			= false;


	if(!pOutfit && CurrentIItem()->GetSlot()!=NO_ACTIVE_SLOT && !m_pInv->m_slots[CurrentIItem()->GetSlot()].m_bPersistent && m_pInv->CanPutInSlot(CurrentIItem()))
	{
		UIPropertiesBox.AddItem(getComplexString("st_move_to_slot",CurrentIItem()).c_str(),  nullptr, INVENTORY_TO_SLOT_ACTION);
		b_show			= true;
	}
	if(CurrentIItem()->Belt() && m_pInv->CanPutInBelt(CurrentIItem()))
	{
		UIPropertiesBox.AddItem(getComplexString("st_move_on_belt",CurrentIItem()).c_str(),  nullptr, INVENTORY_TO_BELT_ACTION);
		b_show			= true;
	}

	if(CurrentIItem()->Ruck() && m_pInv->CanPutInRuck(CurrentIItem()) && (CurrentIItem()->GetSlot()==size_t(-1) || !m_pInv->m_slots[CurrentIItem()->GetSlot()].m_bPersistent) )
	{
		if(!pOutfit)
			UIPropertiesBox.AddItem(getComplexString("st_move_to_bag",CurrentIItem()).c_str(),  nullptr, INVENTORY_TO_BAG_ACTION);
		else
			UIPropertiesBox.AddItem(getComplexString("st_undress_outfit",pOutfit).c_str(),  nullptr, INVENTORY_TO_BAG_ACTION);
		bAlreadyDressed = true;
		b_show			= true;
	}
	if(pOutfit  && !bAlreadyDressed )
	{
		UIPropertiesBox.AddItem(getComplexString("st_dress_outfit",pOutfit).c_str(),  nullptr, INVENTORY_TO_SLOT_ACTION);
		b_show			= true;
	}
	
	//отсоединение аддонов от вещи
	if(pWeapon)
	{
		if(pWeapon->GrenadeLauncherAttachable() && pWeapon->IsGrenadeLauncherAttached())
		{
			UIPropertiesBox.AddItem(getComplexString("st_detach_gl",nullptr,getAddonInvName(pWeapon->GetGrenadeLauncherName().c_str())).c_str(),  nullptr, INVENTORY_DETACH_GRENADE_LAUNCHER_ADDON);
		b_show			= true;
		}
		if(pWeapon->ScopeAttachable() && pWeapon->IsScopeAttached())
		{
			UIPropertiesBox.AddItem(getComplexString("st_detach_scope",nullptr,getAddonInvName(pWeapon->GetScopeName().c_str())).c_str(),  nullptr, INVENTORY_DETACH_SCOPE_ADDON);
		b_show			= true;
		}
		if(pWeapon->SilencerAttachable() && pWeapon->IsSilencerAttached())
		{
			UIPropertiesBox.AddItem(getComplexString("st_detach_silencer",nullptr,getAddonInvName(pWeapon->GetSilencerName().c_str())).c_str(),  nullptr, INVENTORY_DETACH_SILENCER_ADDON);
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
					pWeapon		= smart_cast<CWeaponMagazined*>((CWeapon*)itm->Child(i)->m_pData);
					if(pWeapon->GetAmmoElapsed())
					{
						b = true;
						break;
					}
				}
			}

			if(b){
				UIPropertiesBox.AddItem(getComplexString("st_unload_magazine",pWeapon).c_str(),  nullptr, INVENTORY_UNLOAD_MAGAZINE);
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
			UIPropertiesBox.AddItem(getComplexString("st_attach_scope_to_pistol",pScope).c_str(),  static_cast<void*>(tgt), INVENTORY_ATTACH_ADDON);
			b_show			= true;
		 }
		 if(m_pInv->m_slots[RIFLE_SLOT].m_pIItem != NULL &&
			m_pInv->m_slots[RIFLE_SLOT].m_pIItem->CanAttach(pScope))
		 {
			PIItem tgt = m_pInv->m_slots[RIFLE_SLOT].m_pIItem;
			UIPropertiesBox.AddItem(getComplexString("st_attach_scope_to_rifle",pScope).c_str(),  static_cast<void*>(tgt), INVENTORY_ATTACH_ADDON);
			b_show			= true;
		 }
	}
	else if(pSilencer)
	{
		 if(m_pInv->m_slots[PISTOL_SLOT].m_pIItem != NULL &&
		   m_pInv->m_slots[PISTOL_SLOT].m_pIItem->CanAttach(pSilencer))
		 {
			PIItem tgt = m_pInv->m_slots[PISTOL_SLOT].m_pIItem;
			UIPropertiesBox.AddItem(getComplexString("st_attach_silencer_to_pistol",pSilencer).c_str(),  static_cast<void*>(tgt), INVENTORY_ATTACH_ADDON);
			b_show			= true;
		 }
		 if(m_pInv->m_slots[RIFLE_SLOT].m_pIItem != NULL &&
			m_pInv->m_slots[RIFLE_SLOT].m_pIItem->CanAttach(pSilencer))
		 {
			PIItem tgt = m_pInv->m_slots[RIFLE_SLOT].m_pIItem;
			UIPropertiesBox.AddItem(getComplexString("st_attach_silencer_to_rifle",pSilencer).c_str(),  static_cast<void*>(tgt), INVENTORY_ATTACH_ADDON);
			b_show			= true;
		 }
	}
	else if(pGrenadeLauncher)
	{
		 if(m_pInv->m_slots[RIFLE_SLOT].m_pIItem != NULL &&
			m_pInv->m_slots[RIFLE_SLOT].m_pIItem->CanAttach(pGrenadeLauncher))
		 {
			PIItem tgt = m_pInv->m_slots[RIFLE_SLOT].m_pIItem;
			UIPropertiesBox.AddItem(getComplexString("st_attach_gl_to_rifle",pGrenadeLauncher).c_str(),  static_cast<void*>(tgt), INVENTORY_ATTACH_ADDON);
			b_show			= true;
		 }

	}
	LPCSTR _action = NULL;

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
			CurrentItem()->SetMask(nullptr);
			ToSlot(CurrentItem(), true);
			break;
		case INVENTORY_TO_BELT_ACTION:	
			CurrentItem()->SetMask(nullptr);
			ToBelt(CurrentItem(),false);
			break;
		case INVENTORY_TO_BAG_ACTION:	
			CurrentItem()->SetMask(nullptr);
			ToBag(CurrentItem(),false);
			break;
		case INVENTORY_DROP_ACTION:
			{
				CurrentItem()->SetMask(nullptr);
				void* d = UIPropertiesBox.GetClickedItem()->GetData();
				bool b_all = (d==(void*)33);

				DropCurrentItem(b_all);
			}break;
		case INVENTORY_EAT_ACTION:
			CurrentItem()->SetMask(nullptr);
			EatItem(CurrentIItem());
			break;
		case INVENTORY_ATTACH_ADDON:
			CurrentItem()->SetMask(nullptr);
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
		case INVENTORY_UNLOAD_MAGAZINE:
			{
				CUICellItem * itm = CurrentItem();
				(smart_cast<CWeaponMagazined*>((CWeapon*)itm->m_pData))->UnloadMagazine();
				for(size_t i=0; i<itm->ChildsCount(); ++i)
				{
					CUICellItem * child_itm			= itm->Child(i);
					(smart_cast<CWeaponMagazined*>((CWeapon*)child_itm->m_pData))->UnloadMagazine();
				}
			}break;
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
