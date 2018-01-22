#include "pch_script.h"
#include "inventory.h"
#include "actor.h"
#include "trade.h"
#include "weapon.h"

#include "ui/UIInventoryUtilities.h"

#include "eatable_item.h"
#include "script_engine.h"
#include "xrmessages.h"
//#include "game_cl_base.h"
#include "xr_level_controller.h"
#include "level.h"
#include "ai_space.h"
#include "entitycondition.h"
#include "game_base_space.h"
#include "clsid_game.h"
#include "game_object_space.h"
#include "script_callback_ex.h"
#include "item_place_change_enum.h"
#include "UIGameSP.h"

using namespace InventoryUtilities;

// what to block
u32	INV_STATE_BLOCK_ALL = 0xffffffff;
//u32	INV_STATE_LADDER		= (1<<RIFLE_SLOT)+(1<<SHOTGUN_SLOT)+ (1 << APPARATUS_SLOT);
u32	INV_STATE_LADDER = INV_STATE_BLOCK_ALL;
u32	INV_STATE_CAR = INV_STATE_LADDER;
u32	INV_STATE_INV_WND = INV_STATE_BLOCK_ALL;
u32	INV_STATE_BUY_MENU = INV_STATE_BLOCK_ALL;

CInventorySlot::CInventorySlot() 
{
	m_pIItem				= nullptr;
	m_bVisible				= true;
	m_bPersistent			= false;
	m_blockCounter			= 0;
}

CInventorySlot::~CInventorySlot() 
{
}

bool CInventorySlot::CanBeActivated() const 
{
	return (m_bVisible && !IsBlocked());
};

bool CInventorySlot::IsBlocked() const 
{
	return (m_blockCounter>0);
}

CInventory::CInventory() 
{
	m_fTakeDist									= pSettings->r_float	("inventory","take_dist");
	m_fMaxWeight								= pSettings->r_float	("inventory","max_weight");
	m_iMaxBelt									= pSettings->r_s32		("inventory","max_belt");
	
	m_slots.resize								(SLOTS_TOTAL);
	
	m_iActiveSlot								= NO_ACTIVE_SLOT;
	m_iNextActiveSlot							= NO_ACTIVE_SLOT;
	m_iPrevActiveSlot							= NO_ACTIVE_SLOT;
	m_iLoadActiveSlot							= NO_ACTIVE_SLOT;
	m_ActivationSlotReason						= eGeneral;
	m_pTarget									= nullptr;
	m_bForceRecalcAmmos=true;
	string256 temp;
	for(u32 i=0; i<m_slots.size(); ++i ) 
	{
		sprintf_s(temp, "slot_persistent_%d", i+1);
		if(pSettings->line_exist("inventory",temp)) 
			m_slots[i].m_bPersistent = !!pSettings->r_bool("inventory",temp);
	};

	m_slots[PDA_SLOT].m_bVisible				= false;
	m_slots[OUTFIT_SLOT].m_bVisible				= false;
	m_slots[ARTEFACT_SLOT].m_bVisible			= false;
	m_slots[TORCH_SLOT].m_bVisible				= false;
	m_slots[DETECTOR_ARTS_SLOT].m_bVisible		= false;
	m_slots[DETECTOR_ANOM_SLOT].m_bVisible		= false;
	m_slots[PNV_SLOT].m_bVisible				= false;
	m_slots[BIODEV_SLOT].m_bVisible				= false;

	m_bSlotsUseful								= true;
	m_bBeltUseful								= false;

	m_fTotalWeight								= -1.f;
	m_dwModifyFrame								= 0;
	m_drop_last_frame							= false;
	m_iLoadActiveSlotFrame						= u32(-1);
}


CInventory::~CInventory() 
{
}

void CInventory::Clear()
{
	m_all.clear							();
	m_ruck.clear						();
	m_belt.clear						();
	m_apItems.clear();
	for(u32 i=0; i<m_slots.size(); i++)
	{
		m_slots[i].m_pIItem				= nullptr;
	}
	

	m_pOwner							= nullptr;

	CalcTotalWeight						();
	InvalidateState						();
}

void CInventory::Take(CGameObject *pObj, bool bNotActivate, bool strict_placement)
{
	CInventoryItem *pIItem				= smart_cast<CInventoryItem*>(pObj);
	VERIFY								(pIItem);
	if (pIItem->m_pCurrentInventory == this) return;
	

	if(pIItem->m_pCurrentInventory)
	{
		Msg("! ERROR CInventory::Take but object has m_pCurrentInventory");
		Msg("! Inventory Owner is [%d]", GetOwner()->object_id());
		Msg("! Object Inventory Owner is [%d]", pIItem->m_pCurrentInventory->GetOwner()->object_id());

		CObject* p	= pObj->H_Parent();
		if(p)
			Msg("! object parent is [%s] [%d]", p->cName().c_str(), p->ID());
	}

	R_ASSERT							(CanTakeItem(pIItem));
	
	pIItem->m_pCurrentInventory			= this;
	pIItem->SetDropManual				(FALSE);

	m_all.push_back						(pIItem);
	if (pIItem->GetAPRadiation() != 0)
		m_apItems.push_back(pIItem);

	if(!strict_placement)
		pIItem->m_eItemPlace			= eItemPlaceUndefined;

	bool result							= false;
	switch(pIItem->m_eItemPlace)
	{
	case eItemPlaceBelt:
		result							= Belt(pIItem); 
#ifdef DEBUG
		if(!result) 
			Msg("cant put in belt item %s", *pIItem->object().cName());
#endif

		break;
	case eItemPlaceRuck:
		result							= Ruck(pIItem);
#ifdef DEBUG
		if(!result) 
			Msg("cant put in ruck item %s", *pIItem->object().cName());
#endif

		break;
	case eItemPlaceSlot:
		result							= Slot(pIItem, bNotActivate); 
#ifdef DEBUG
		if(!result) 
			Msg("cant slot in ruck item %s", *pIItem->object().cName());
#endif

		break;
	default:
		if(CanPutInSlot(pIItem))
		{
			result						= Slot(pIItem, bNotActivate); VERIFY(result);
		} 
		else if ( !pIItem->RuckDefault() && CanPutInBelt(pIItem))
		{
			result						= Belt(pIItem); VERIFY(result);
		}
		else
		{
			result						= Ruck(pIItem); VERIFY(result);
		}
	}
	
	m_pOwner->OnItemTake				(pIItem);

	CalcTotalWeight						();
	InvalidateState						();

	pIItem->object().processing_deactivate();
	VERIFY								(pIItem->m_eItemPlace != eItemPlaceUndefined);
}

bool CInventory::DropItem(CGameObject *pObj) 
{
	CInventoryItem *pIItem				= smart_cast<CInventoryItem*>(pObj);
	VERIFY								(pIItem);
	if( !pIItem )						return false;
	if (!pIItem->m_pCurrentInventory)	return false;

	if(pIItem->m_pCurrentInventory!=this)
	{
		Msg("ahtung !!! [%d]", Device.dwFrame);
		Msg("CInventory::DropItem pIItem->m_pCurrentInventory!=this");
		Msg("this = [%d]", GetOwner()->object_id());
		Msg("pIItem->m_pCurrentInventory = [%d]", pIItem->m_pCurrentInventory->GetOwner()->object_id());
		Msg("Obj section = [%s]", pIItem->object().cNameSect().c_str());
		Msg("Obj ID = [%d]", pIItem->object().ID());
	}

	R_ASSERT							(pIItem->m_pCurrentInventory);
	R_ASSERT							(pIItem->m_pCurrentInventory==this);
	VERIFY								(pIItem->m_eItemPlace!=eItemPlaceUndefined);

	CGameObject &obj = pIItem->object();
	obj.processing_activate(); 
	
	switch(pIItem->m_eItemPlace)
	{
	case eItemPlaceUndefined:{
		Msg("! #ERROR: item %s place in undefined in inventory ", obj.Name_script());
		}break;
	case eItemPlaceBelt:{
			if (InBelt(pIItem)) {
			m_belt.erase(std::find(m_belt.begin(), m_belt.end(), pIItem));
			obj.processing_deactivate();
		} 
		 else
		     Msg("! #ERROR: item %s place==belt, but m_belt not contains item", obj.Name_script());
		}break;
	case eItemPlaceRuck:{
		if (InRuck(pIItem))
		{
			m_ruck.erase(std::find(m_ruck.begin(), m_ruck.end(), pIItem));
		}
		else
			Msg("! #ERROR: item %s place==ruck, but m_ruck not contains item", obj.Name_script());
		}break;
	case eItemPlaceSlot:{
			R_ASSERT			(InSlot(pIItem));
			if(m_iActiveSlot == pIItem->GetSlot()) 
				Activate	(NO_ACTIVE_SLOT);

			m_slots[pIItem->GetSlot()].m_pIItem = nullptr;							
			pIItem->object().processing_deactivate();
		}break;
	default:
		NODEFAULT;
	};

	TIItemContainer::iterator	apItem = std::find(m_apItems.begin(), m_apItems.end(), pIItem);
	if (apItem != m_apItems.end())
		m_apItems.erase(apItem);
			
	TIItemContainer::iterator	it = std::find(m_all.begin(), m_all.end(), pIItem);
	if ( it != m_all.end())
		m_all.erase				(it);
	else
		Msg						("! CInventory::Drop item not found in inventory!!!");
	pIItem->OnMoveToDrop();
	pIItem->m_pCurrentInventory = nullptr;

	m_pOwner->OnItemDrop			(smart_cast<CInventoryItem*>(pObj));

	CalcTotalWeight					();
	InvalidateState					();
	m_drop_last_frame				= true;
	return							true;
}

void callMovingCallback(CInventoryItem* item,InventoryPlaceChange::EnumItemPlaceChange placeChangeId)
{
	CObject* parent=item->object().H_Parent();
	if (parent)
	{
		CEntityAlive* callParent=smart_cast<CEntityAlive*>(parent);
		if (callParent)
		{
			callParent->callback(GameObject::ECallbackType::OnInventoryItemPlaceChange)(item->object().lua_game_object(),placeChangeId);
		}
	}
}
//положить вещь в слот
bool CInventory::Slot(PIItem pIItem, bool bNotActivate) 
{
	VERIFY(pIItem);
//	Msg("To Slot %s[%d]", *pIItem->object().cName(), pIItem->object().ID());
	
	if(!CanPutInSlot(pIItem)) 
	{
#if 0//def _DEBUG
		Msg("there is item %s[%d,%x] in slot %d[%d,%x]", 
				*m_slots[pIItem->GetSlot()].m_pIItem->object().cName(), 
				m_slots[pIItem->GetSlot()].m_pIItem->object().ID(), 
				m_slots[pIItem->GetSlot()].m_pIItem, 
				pIItem->GetSlot(), 
				pIItem->object().ID(),
				pIItem);
#endif
		if(m_slots[pIItem->GetSlot()].m_pIItem == pIItem && !bNotActivate )
			Activate(pIItem->GetSlot());

		return false;
	}


	m_slots[pIItem->GetSlot()].m_pIItem = pIItem;

	//удалить из рюкзака или пояса
	TIItemContainer::iterator it = std::find(m_ruck.begin(), m_ruck.end(), pIItem);
	if(m_ruck.end() != it) 
	{
			m_ruck.erase(it);
			callMovingCallback(pIItem,InventoryPlaceChange::removeFromRuck);
	}

	it = std::find(m_belt.begin(), m_belt.end(), pIItem);
	if(m_belt.end() != it) 
	{
			m_belt.erase(it);
			callMovingCallback(pIItem,InventoryPlaceChange::removeFromBelt);
	}



	if (( (m_iActiveSlot==pIItem->GetSlot())||(m_iActiveSlot==NO_ACTIVE_SLOT) && m_iNextActiveSlot==NO_ACTIVE_SLOT) && (!bNotActivate))
		Activate				(pIItem->GetSlot());

	
	m_pOwner->OnItemSlot		(pIItem, pIItem->m_eItemPlace);
	pIItem->m_eItemPlace		= eItemPlaceSlot;
	pIItem->OnMoveToSlot		();
	
	pIItem->object().processing_activate();

	callMovingCallback(pIItem,InventoryPlaceChange::putToSlot);
	return						true;
}

bool CInventory::Belt(PIItem pIItem) 
{
	if(!CanPutInBelt(pIItem))
	{
		if (pIItem->m_eItemPlace==EItemPlace::eItemPlaceBelt && !pIItem->Belt())
		{
			CGameObject &obj = pIItem->object();
			Msg("~ WARNING: item [%s] in belt, but configured for unplaced in belt.Put in ruck.",obj.Name_script());
			Ruck(pIItem);
		}
		return false;
	}
	
	//вещь была в слоте
	bool in_slot = InSlot(pIItem);
	if(in_slot) 
	{
		if(m_iActiveSlot == pIItem->GetSlot()) Activate(NO_ACTIVE_SLOT);
		m_slots[pIItem->GetSlot()].m_pIItem = nullptr;
		callMovingCallback(pIItem,InventoryPlaceChange::removeFromSlot);
	}
	
	m_belt.insert(m_belt.end(), pIItem); 

	if(!in_slot)
	{
		TIItemContainer::iterator it = std::find(m_ruck.begin(), m_ruck.end(), pIItem); 
		if(m_ruck.end() != it) 
		{
				m_ruck.erase(it);
				callMovingCallback(pIItem,InventoryPlaceChange::removeFromRuck);
		}
	}

	CalcTotalWeight();
	InvalidateState						();

	EItemPlace p = pIItem->m_eItemPlace;
	pIItem->m_eItemPlace = eItemPlaceBelt;
	m_pOwner->OnItemBelt(pIItem, p);
	pIItem->OnMoveToBelt();

	if(in_slot)
		pIItem->object().processing_deactivate();
	pIItem->object().processing_activate();
	callMovingCallback(pIItem,InventoryPlaceChange::putToBelt);
	return true;
}

bool CInventory::Ruck(PIItem pIItem) 
{
	if(!CanPutInRuck(pIItem)) return false;	
	bool in_slot = InSlot(pIItem);
	//вещь была в слоте
	if(in_slot) 
	{
		if(m_iActiveSlot == pIItem->GetSlot()) Activate(NO_ACTIVE_SLOT);
		m_slots[pIItem->GetSlot()].m_pIItem = nullptr;
		callMovingCallback(pIItem,InventoryPlaceChange::removeFromSlot);
	}
	else
	{
		//вещь была на поясе или вообще только поднята с земли
		TIItemContainer::iterator it = std::find(m_belt.begin(), m_belt.end(), pIItem); 
		if(m_belt.end() != it) 
		{
			m_belt.erase(it);
			callMovingCallback(pIItem,InventoryPlaceChange::removeFromBelt);
		}
	}
	

	m_ruck.insert									(m_ruck.end(), pIItem); 
	CalcTotalWeight									();
	InvalidateState									();

	m_pOwner->OnItemRuck							(pIItem, pIItem->m_eItemPlace);
	pIItem->OnMoveToRuck							();
	pIItem->m_eItemPlace = eItemPlaceRuck;

	if(in_slot)
		pIItem->object().processing_deactivate();
	callMovingCallback(pIItem,InventoryPlaceChange::putToRuck);
	return true;
}

void CInventory::Activate_deffered	(u32 slot, u32 _frame)
{
	 m_iLoadActiveSlot			= slot;
	 m_iLoadActiveSlotFrame		= _frame;
}

PIItem CInventory::ActiveItem(int flag) const
{
	return m_iActiveSlot == NO_ACTIVE_SLOT ? nullptr : m_slots[m_iActiveSlot].m_pIItem;
}

void  CInventory::ActivateNextItemInActiveSlot()
{
	if(m_iActiveSlot==NO_ACTIVE_SLOT)	return;
	
	PIItem current_item		= m_slots[m_iActiveSlot].m_pIItem;
	PIItem new_item			= nullptr;

	bool b = (current_item==nullptr);
	
	TIItemContainer::const_iterator it		= m_all.begin();
	TIItemContainer::const_iterator it_e	= m_all.end();

	for(; it!=it_e; ++it) 
	{
		PIItem _pIItem		= *it;
		if(_pIItem==current_item)
		{
			b = true;
			continue;
		}
		if(_pIItem->GetSlot()==m_iActiveSlot)
			new_item = _pIItem;

		if(b && new_item)
			break;
	}

	if(new_item==nullptr)
		return; //only 1 item for this slot

	bool res = Ruck						(current_item);
	R_ASSERT							(res);
	NET_Packet							P;
	current_item->object().u_EventGen	(P, GEG_PLAYER_ITEM2RUCK, current_item->object().H_Parent()->ID());
	P.w_u16								(current_item->object().ID());
	current_item->object().u_EventSend	(P);

	res = Slot							(new_item);
	R_ASSERT							(res);
	new_item->object().u_EventGen		(P, GEG_PLAYER_ITEM2SLOT, new_item->object().H_Parent()->ID());
	P.w_u16								(new_item->object().ID());
	new_item->object().u_EventSend		(P);
	
	//activate
	new_item->object().u_EventGen		(P, GEG_PLAYER_ACTIVATE_SLOT, new_item->object().H_Parent()->ID());
	P.w_u32								(new_item->GetSlot());
	new_item->object().u_EventSend		(P);


	Msg("CHANGE");
}

bool CInventory::Activate(u32 slot, EActivationReason reason, bool bForce) 
{	
	if(	m_ActivationSlotReason==eKeyAction	&& reason==eImportUpdate )
		return false;

	bool res = false;
	 
	if(Device.dwFrame == m_iLoadActiveSlotFrame) 
	{
		 if( (m_iLoadActiveSlot == slot) && m_slots[slot].m_pIItem )
			m_iLoadActiveSlotFrame = u32(-1);
		 else
			{
			 res = false;
			 goto _finish;
			}

	}

	if( (slot!=NO_ACTIVE_SLOT && m_slots[slot].IsBlocked()) && !bForce)
	{
		res = false;
		goto _finish;
	}

	R_ASSERT2(slot == NO_ACTIVE_SLOT || slot<m_slots.size(), "wrong slot number");

	if(slot != NO_ACTIVE_SLOT && !m_slots[slot].m_bVisible) 
	{
		res = false;
		goto _finish;
	}
	
	if(m_iActiveSlot == slot && m_iActiveSlot != NO_ACTIVE_SLOT && m_slots[m_iActiveSlot].m_pIItem)
	{
		m_slots[m_iActiveSlot].m_pIItem->Activate();
	}

	if(	m_iActiveSlot == slot || 
		(m_iNextActiveSlot == slot &&
		 m_iActiveSlot != NO_ACTIVE_SLOT &&
		 m_slots[m_iActiveSlot].m_pIItem &&
		 m_slots[m_iActiveSlot].m_pIItem->IsHiding()
		 )
	   )
	{
		res = false;
		goto _finish;
	}

	//активный слот не выбран
	if(m_iActiveSlot == NO_ACTIVE_SLOT)
	{
		if(m_slots[slot].m_pIItem)
		{
			m_iNextActiveSlot		= slot;
			m_ActivationSlotReason	= reason;
			res = true;
			goto _finish;
		}
		else 
		{
			if(slot==GRENADE_SLOT)//fake for grenade
			{
				PIItem gr = SameSlot(GRENADE_SLOT, nullptr, true);
				if(gr)
				{
					Slot(gr);
					goto _finish;
				}else
				{
					res = false;
					goto _finish;
				}

			}else
			{
				res = false;
				goto _finish;
			}
		}
	}
	//активный слот задействован
	else if(slot == NO_ACTIVE_SLOT || m_slots[slot].m_pIItem)
	{
		if(m_slots[m_iActiveSlot].m_pIItem)
			m_slots[m_iActiveSlot].m_pIItem->Deactivate();

		m_iNextActiveSlot		= slot;
		m_ActivationSlotReason	= reason;
	
		res = true;
		goto _finish;
	}

	_finish:

	if(res)
		m_ActivationSlotReason	= reason;
	return res;
}


PIItem CInventory::ItemFromSlot(u32 slot) const
{
	VERIFY(NO_ACTIVE_SLOT != slot);
	return m_slots[slot].m_pIItem;
}

void CInventory::SendActionEvent(s32 cmd, u32 flags) 
{
	CActor *pActor = smart_cast<CActor*>(m_pOwner);
	if (!pActor) return;

	NET_Packet		P;
	pActor->u_EventGen		(P,GE_INV_ACTION, pActor->ID());
	P.w_s32					(cmd);
	P.w_u32					(flags);
	P.w_s32					(pActor->GetZoomRndSeed());
	P.w_s32					(pActor->GetShotRndSeed());
	pActor->u_EventSend		(P, net_flags(TRUE, TRUE, FALSE, TRUE));
};


bool CInventory::ProcessSlotAction (bool flag,u32 slotId)
{
	bool b_send_event=false;
	if(flag)
	{
		if (m_iActiveSlot==slotId && m_slots[m_iActiveSlot].m_pIItem)
		{
			if(IsGameTypeSingle())
				b_send_event = Activate(NO_ACTIVE_SLOT);
			else
				ActivateNextItemInActiveSlot();
		}
		else
		{
			if (m_iActiveSlot == slotId && !IsGameTypeSingle())
				return false;
			b_send_event = Activate(slotId, eKeyAction);
		}
	}
	return b_send_event;
}

bool CInventory::Action(s32 cmd, u32 flags) 
{
	CActor *pActor = smart_cast<CActor*>(m_pOwner);
	
	if (pActor)
	{
		switch(cmd)
		{
			case kWPN_FIRE:
			{
				pActor->SetShotRndSeed();
			}break;
			case kWPN_ZOOM : 
			{
				pActor->SetZoomRndSeed();
			}break;
		};
	};

	if (g_pGameLevel && OnClient() && pActor) {
		switch(cmd)
		{
		case kUSE:
			{
			}break;
		
		case kDROP:		
		
			{
				SendActionEvent(cmd, flags);
				return true;
			}break;

		case kWPN_NEXT:
		case kWPN_RELOAD:
		case kWPN_FIRE:
		case kWPN_FUNC:
		case kWPN_FIREMODE_NEXT:
		case kWPN_FIREMODE_PREV:
		case kWPN_ZOOM : 
		case kTORCH:
		case kNIGHT_VISION:
			{
				SendActionEvent(cmd, flags);
			}break;
		}
	}


	if (m_iActiveSlot < m_slots.size() && m_slots[m_iActiveSlot].m_pIItem && m_slots[m_iActiveSlot].m_pIItem->Action(cmd, flags)) 
		return true;
	bool b_send_event = false;
	switch(cmd) 
	{
	case kWPN_1:
		b_send_event=ProcessSlotAction(flags&CMD_START,KNIFE_SLOT);
		break;
	case kWPN_2:
		b_send_event=ProcessSlotAction(flags&CMD_START,PISTOL_SLOT);
		break;
	case kWPN_3:
		b_send_event=ProcessSlotAction(flags&CMD_START,RIFLE_SLOT);
		break;
	case kWPN_4:
		b_send_event=ProcessSlotAction(flags&CMD_START,SHOTGUN_SLOT);
		break;
	case kWPN_5:
		b_send_event=ProcessSlotAction(flags&CMD_START,GRENADE_SLOT);
		break;
	case kWPN_6:
		b_send_event=ProcessSlotAction(flags&CMD_START,APPARATUS_SLOT);
		break;
	case kWPN_7:
		b_send_event=ProcessSlotAction(flags&CMD_START,BOLT_SLOT);
		break;
	case kWPN_8:
	   {
		   //not used slot. maybe in future
		}break;
	case kARTEFACT:
		{
			if(flags&CMD_START)
			{
				if(static_cast<int>(m_iActiveSlot) == ARTEFACT_SLOT && m_slots[m_iActiveSlot].m_pIItem && IsGameTypeSingle())
				{
					b_send_event = Activate(NO_ACTIVE_SLOT);
				}else {
					b_send_event = Activate(ARTEFACT_SLOT);
				}
			}
		}break;
	}

	if(b_send_event && g_pGameLevel && OnClient() && pActor)
			SendActionEvent(cmd, flags);

	return false;
}


void CInventory::Update() 
{
	bool bActiveSlotVisible;
	
	/*PIItem itm=nullptr;
	int total = static_cast<int>(m_all.size());
	int it    = total - 1;
	while (it >= 0)
	{
		itm = m_all[it];
		if (!itm || NULL == itm->m_object)
		{
			Msg("! ERROR: detected bad object %s in inventory m_all[%d/%d]", (itm ? itm->Name() : "(NULL)"), it, total);
			m_all.erase(m_all.begin() + it);			
		}
		else it--;
	} */

	if(m_iActiveSlot == NO_ACTIVE_SLOT || 
		!m_slots[m_iActiveSlot].m_pIItem ||
		m_slots[m_iActiveSlot].m_pIItem->IsHidden())
	{ 
		bActiveSlotVisible = false;
	}
	else 
	{
		bActiveSlotVisible = true;
	}

	if(m_iNextActiveSlot != m_iActiveSlot && !bActiveSlotVisible)
	{
		if(m_iNextActiveSlot != NO_ACTIVE_SLOT && m_slots[m_iNextActiveSlot].m_pIItem)
			m_slots[m_iNextActiveSlot].m_pIItem->Activate();

		m_iActiveSlot = m_iNextActiveSlot;
	}


	UpdateDropTasks	();
}

void CInventory::UpdateDropTasks()
{
	for(u32 i=0; i<m_slots.size(); ++i)	
	{
		if(m_slots[i].m_pIItem)
			UpdateDropItem		(m_slots[i].m_pIItem);
	}

	for(int i = 0; i < 2; ++i)	
	{
		TIItemContainer &list			= i?m_ruck:m_belt;
		auto it		= list.begin();
		auto it_e	= list.end();
	
		while (it != it_e)
		{
			auto _it = it++;
			if (*_it)
				UpdateDropItem(*_it);
			else
			{
				Msg("! ERROR: unassigned item in inventory.%s", (i == 0 ? "belt" : "slot"));
				list.erase(_it);
				TIItemContainer::iterator	apItem = std::find(m_apItems.begin(), m_apItems.end(), *_it);
				if (apItem != m_apItems.end())
					m_apItems.erase(apItem);
			}
		}

	}

	if (m_drop_last_frame)
	{
		m_drop_last_frame			= false;
		m_pOwner->OnItemDropUpdate	();
	}
}

void CInventory::UpdateDropItem(PIItem pIItem)
{
	if (!pIItem->m_object)
	{
		LPCSTR name = pIItem->Name();
		Msg("!ERROR invalid CPhysicsShellHolder data for [%s]", name? name : "unknown");
		FATAL("ENGINE crash! See log for detail!");
	}
	if(pIItem->m_object && pIItem->GetDropManual() )
	{
		pIItem->SetDropManual(FALSE);
		if ( OnServer() ) 
		{
			NET_Packet					P;
			pIItem->object().u_EventGen	(P, GE_OWNERSHIP_REJECT, pIItem->object().H_Parent()->ID());
			P.w_u16						(u16(pIItem->object().ID()));
			pIItem->object().u_EventSend(P);

			//CObject *parent = pIItem->object().H_Parent();
			//Fvector &pos = parent->Position();
			//Fvector  dir = parent->Direction();
			//dir.y = 0;
			//dir.normalize();
			//static float coef = 4.0f;
			//CActor *owner = smart_cast<CActor*>(parent);
			//if (owner)
			//{
			//	auto &R = owner->Orientation();
			//	dir.setHP(R.yaw, R.pitch);
			//	float dist = 0.9f;// owner->GetDropPower() * coef + 0.5f;
			//	dir.mul(dist);
			//}
			//if (pIItem->object().Visual())
			//	dir.mul(0.7f + pIItem->object().Radius() /** 2.0f*/);
			//pos.add(dir);
			//pos.y = pos.y + 0.3f;
			//pIItem->object().ChangePosition(pos);
			//pIItem->m_dropTarget = pos;
		}
	}// dropManual
}

//ищем на поясе гранату такоже типа
PIItem CInventory::Same(const PIItem pIItem, bool bSearchRuck) const
{
	const TIItemContainer &list = bSearchRuck ? m_ruck : m_belt;

	for(TIItemContainer::const_iterator it = list.begin(); list.end() != it; ++it) 
	{
		const PIItem l_pIItem = *it;
		
		if((l_pIItem != pIItem) && 
				!xr_strcmp(l_pIItem->object().cNameSect(), 
				pIItem->object().cNameSect())) 
			return l_pIItem;
	}
	return nullptr;
}

//ищем на поясе вещь для слота 

PIItem CInventory::SameSlot(const u32 slot, PIItem pIItem, bool bSearchRuck) const
{
	if(slot == NO_ACTIVE_SLOT) 	return nullptr;

	const TIItemContainer &list = bSearchRuck ? m_ruck : m_belt;
	
	for(TIItemContainer::const_iterator it = list.begin(); list.end() != it; ++it) 
	{
		PIItem _pIItem = *it;
		if(_pIItem != pIItem && _pIItem->GetSlot() == slot) return _pIItem;
	}

	return nullptr;
}

PIItem CInventory::Get(shared_str name, bool bSearchRuck) const
{
	return Get(name.c_str(), bSearchRuck);
}
//найти в инвенторе вещь с указанным именем
PIItem CInventory::Get(const char *name, bool bSearchRuck) const
{
	const TIItemContainer &list = bSearchRuck ? m_ruck : m_belt;
	
	for(TIItemContainer::const_iterator it = list.begin(); list.end() != it; ++it) 
	{
		PIItem pIItem = *it;
		if(pIItem && !xr_strcmp(pIItem->object().cNameSect(), name) && 
								pIItem->Useful()) 
				return pIItem;
	}
	return nullptr;
}

PIItem CInventory::Get(CLASS_ID cls_id, bool bSearchRuck) const
{
	const TIItemContainer &list = bSearchRuck ? m_ruck : m_belt;
	
	for(TIItemContainer::const_iterator it = list.begin(); list.end() != it; ++it) 
	{
		PIItem pIItem = *it;
		if(pIItem && pIItem->object().CLS_ID == cls_id && 
								pIItem->Useful()) 
				return pIItem;
	}
	return nullptr;
}

PIItem CInventory::Get(const u16 id, bool bSearchRuck) const
{
	const TIItemContainer &list = bSearchRuck ? m_ruck : m_belt;

	for(TIItemContainer::const_iterator it = list.begin(); list.end() != it; ++it) 
	{
		PIItem pIItem = *it;
		if(pIItem && pIItem->object().ID() == id) 
			return pIItem;
	}
	return nullptr;
}

//search both (ruck and belt)
PIItem CInventory::GetAny(const char *name) const
{
	PIItem itm = Get(name, false);
	if(!itm)
		itm = Get(name, true);
	return itm;
}

PIItem CInventory::item(CLASS_ID cls_id) const
{
	const TIItemContainer &list = m_all;

	for(TIItemContainer::const_iterator it = list.begin(); list.end() != it; ++it) 
	{
		PIItem pIItem = *it;
		if(pIItem && pIItem->object().CLS_ID == cls_id && 
			pIItem->Useful()) 
			return pIItem;
	}
	return nullptr;
}

float CInventory::TotalWeight() const
{
	VERIFY(m_fTotalWeight>=0.f);
	return m_fTotalWeight;
}


float CInventory::CalcTotalWeight()
{
	float weight = 0;
	for(TIItemContainer::const_iterator it = m_all.begin(); m_all.end() != it; ++it) 
		weight += (*it)->Weight();

	m_fTotalWeight = weight;
	return m_fTotalWeight;
}


u32 CInventory::dwfGetSameItemCount(LPCSTR caSection, bool SearchAll)
{
	u32			l_dwCount = 0;
	TIItemContainer	&l_list = SearchAll ? m_all : m_ruck;
	for(TIItemContainer::iterator l_it = l_list.begin(); l_list.end() != l_it; ++l_it) 
	{
		PIItem	l_pIItem = *l_it;
		if (l_pIItem && !xr_strcmp(l_pIItem->object().cNameSect(), caSection))
			++l_dwCount;
	}
	
	return		(l_dwCount);
}
u32		CInventory::dwfGetGrenadeCount(LPCSTR caSection, bool SearchAll)
{
	u32			l_dwCount = 0;
	TIItemContainer	&l_list = SearchAll ? m_all : m_ruck;
	for(TIItemContainer::iterator l_it = l_list.begin(); l_list.end() != l_it; ++l_it) 
	{
		PIItem	l_pIItem = *l_it;
		if (l_pIItem && l_pIItem->object().CLS_ID == CLSID_GRENADE_F1 || l_pIItem->object().CLS_ID == CLSID_GRENADE_RGD5)
			++l_dwCount;
	}

	return		(l_dwCount);
}

bool CInventory::bfCheckForObject(ALife::_OBJECT_ID tObjectID)
{
	TIItemContainer	&l_list = m_all;
	for(TIItemContainer::iterator l_it = l_list.begin(); l_list.end() != l_it; ++l_it) 
	{
		PIItem	l_pIItem = *l_it;
		if (l_pIItem && l_pIItem->object().ID() == tObjectID)
			return(true);
	}
	return		(false);
}

CInventoryItem *CInventory::get_object_by_id(ALife::_OBJECT_ID tObjectID)
{
	TIItemContainer	&l_list = m_all;
	for(TIItemContainer::iterator l_it = l_list.begin(); l_list.end() != l_it; ++l_it) 
	{
		PIItem	l_pIItem = *l_it;
		if (l_pIItem && l_pIItem->object().ID() == tObjectID)
			return	(l_pIItem);
	}
	return		(0);
}

//скушать предмет 
#include "game_object_space.h"
#include "script_callback_ex.h"
#include "script_game_object.h"
#include "HUDManager.h"
bool CInventory::Eat(PIItem pIItem)
{
	R_ASSERT(pIItem->m_pCurrentInventory==this);
	//устанаовить съедобна ли вещь
	CEatableItem* pItemToEat = smart_cast<CEatableItem*>(pIItem);
	R_ASSERT				(pItemToEat);

	CEntityAlive *entity_alive = smart_cast<CEntityAlive*>(m_pOwner);
	R_ASSERT				(entity_alive);
	
	if (Actor()->get_state()&mcClimb)
	{
		HUD().GetUI()->AddInfoMessage("cant_eat_any");
		HUD().GetUI()->UIGame()->HideShownDialogs();
		return true;
	}

	pItemToEat->UseBy		(entity_alive);

	if(IsGameTypeSingle() && Actor()->m_inventory == this)
		Actor()->callback(GameObject::eUseObject)((smart_cast<CGameObject*>(pIItem))->lua_game_object());

	if(pItemToEat->Empty() && entity_alive->Local())
	{
		NET_Packet					P;
		CGameObject::u_EventGen		(P,GE_OWNERSHIP_REJECT,entity_alive->ID());
		P.w_u16						(pIItem->object().ID());
		CGameObject::u_EventSend	(P);

		CGameObject::u_EventGen		(P,GE_DESTROY,pIItem->object().ID());
		CGameObject::u_EventSend	(P);
		
		callMovingCallback(pIItem,InventoryPlaceChange::removeFromRuck);
		
		return		false;
	}
	return			true;
}

void CInventory::SetActiveSlot(u32 ActiveSlot)
{
	m_iActiveSlot = ActiveSlot;
	m_iNextActiveSlot = ActiveSlot;
}

bool CInventory::InSlot(PIItem pIItem) const
{
	if(pIItem->GetSlot() < m_slots.size() && 
		m_slots[pIItem->GetSlot()].m_pIItem == pIItem)
		return true;
	return false;
}
bool CInventory::InBelt(PIItem pIItem) const
{
	if(Get(pIItem->object().ID(), false)) return true;
	return false;
}
bool CInventory::InRuck(PIItem pIItem) const
{
	if(Get(pIItem->object().ID(), true)) return true;
	return false;
}


bool CInventory::CanPutInSlot(PIItem pIItem) const
{
	if(!m_bSlotsUseful) 
		return false;
	u32 slot = pIItem->GetSlot();
	if (slot==NO_ACTIVE_SLOT) 
		return false;
	if( !GetOwner()->CanPutInSlot(pIItem, slot )) 
		return false;
	if(slot < m_slots.size() && m_slots[slot].m_pIItem == nullptr)
		return true;
	return false;
}
//проверяет можем ли поместить вещь на пояс,
//при этом реально ничего не меняется
bool CInventory::CanPutInBelt(PIItem pIItem)
{
	if(InBelt(pIItem))					return false;
	if(!m_bBeltUseful)					return false;
	if(!pIItem || !pIItem->Belt())		return false;
	if(m_belt.size() == BeltWidth())	return false;

	return FreeRoom_inBelt(m_belt, pIItem, BeltWidth(), 1);
}
//проверяет можем ли поместить вещь в рюкзак,
//при этом реально ничего не меняется
bool CInventory::CanPutInRuck(PIItem pIItem) const
{
	if(InRuck(pIItem)) return false;
	return true;
}

u32	CInventory::dwfGetObjectCount()
{
	return		(m_all.size());
}

CInventoryItem	*CInventory::tpfGetObjectByIndex(int iIndex)
{
	if ((iIndex >= 0) && (iIndex < (int)m_all.size())) {
		TIItemContainer	&l_list = m_all;
		int			i = 0;
		for(TIItemContainer::iterator l_it = l_list.begin(); l_list.end() != l_it; ++l_it, ++i) 
			if (i == iIndex)
				return	(*l_it);
	}
	else {
		ai().script_engine().script_log	(ScriptStorage::eLuaMessageTypeError,"invalid inventory index!");
		return	(0);
	}
	R_ASSERT	(false);
	return		(0);
}

CInventoryItem	*CInventory::GetItemFromInventory(LPCSTR caItemName)
{
	TIItemContainer	&l_list = m_all;

	u32 crc = crc32(caItemName, xr_strlen(caItemName));

	for(TIItemContainer::iterator l_it = l_list.begin(); l_list.end() != l_it; ++l_it)
		if ((*l_it)->object().cNameSect()._get()->dwCRC == crc){
			VERIFY(	0 == xr_strcmp( (*l_it)->object().cNameSect().c_str(), caItemName)  );
			return	(*l_it);
		}
	return	(0);
}


bool CInventory::CanTakeItem(CInventoryItem *inventory_item) const
{
	if (!inventory_item->m_object) return false;
	if (inventory_item->object().getDestroy()) return false;

	if(!inventory_item->CanTake()) return false;

	TIItemContainer::const_iterator it;

	for(it = m_all.begin(); it != m_all.end(); it++)
		if((*it)->object().ID() == inventory_item->object().ID()) break;
	VERIFY3(it == m_all.end(), "item already exists in inventory",*inventory_item->object().cName());

	CActor* pActor = smart_cast<CActor*>(m_pOwner);
	//актер всегда может взять вещь
	if(!pActor && (TotalWeight() + inventory_item->Weight() > m_pOwner->MaxCarryWeight()))
		return	false;

	return	true;
}


u32  CInventory::BeltWidth() const
{
	return m_iMaxBelt;
}

void  CInventory::AddAvailableItems(TIItemContainer& items_container, bool for_trade,bool useBelt,bool useSlots, bool checkVisibility) const
{
	for(TIItemContainer::const_iterator it = m_ruck.begin(); m_ruck.end() != it; ++it) 
	{
		PIItem pIItem = *it;
		if ((!for_trade || pIItem->CanTrade()))
			items_container.push_back(pIItem);
	}

	if(m_bBeltUseful&& useBelt)
	{
		for(TIItemContainer::const_iterator it = m_belt.begin(); m_belt.end() != it; ++it) 
		{
			PIItem pIItem = *it;
			if((!for_trade || pIItem->CanTrade()))
				items_container.push_back(pIItem);
		}
	}
	
	if(m_bSlotsUseful && useSlots)
	{
		TISlotArr::const_iterator slot_it			= m_slots.begin();
		TISlotArr::const_iterator slot_it_e			= m_slots.end();
		for(;slot_it!=slot_it_e;++slot_it)
		{
			const CInventorySlot& S = *slot_it;
			if (S.m_pIItem)
			{
				if ((!for_trade || S.m_pIItem->CanTrade())) 
				{
					if ((!S.m_bPersistent || S.m_pIItem->GetSlot() == GRENADE_SLOT))
						items_container.push_back(S.m_pIItem);
				}
			}
		}
	}		
}

bool CInventory::isBeautifulForActiveSlot	(CInventoryItem *pIItem)
{
	if (!IsGameTypeSingle()) return (true);
	TISlotArr::iterator it =  m_slots.begin();
	for( ; it!=m_slots.end(); ++it) {
		if ((*it).m_pIItem && (*it).m_pIItem->IsNecessaryItem(pIItem))
			return		(true);
	}
	return				(false);
}

#include "WeaponHUD.h"
void CInventory::Items_SetCurrentEntityHud(bool current_entity)
{
	TIItemContainer::iterator it;
	for(it = m_all.begin(); m_all.end() != it; ++it) 
	{
		PIItem pIItem = *it;
		CHudItem* pHudItem = smart_cast<CHudItem*> (pIItem);
		/*if (pHudItem) 
		{
			pHudItem->GetHUD()->Visible(current_entity);
		};*/
		CWeapon* pWeapon = smart_cast<CWeapon*>(pIItem);
		if (pWeapon)
		{
			pWeapon->UpdateAddonsVisibility();
			pWeapon->InitAddons();
		}
	}
};

//call this only via Actor()->SetWeaponHideState()
void CInventory::SetSlotsBlocked(u16 mask, bool bBlock)
{
	bool bChanged = false;
	for(int i =0; i<SLOTS_TOTAL; ++i)
	{
		if(mask & (1<<i))
		{
			bool bCanBeActivated = m_slots[i].CanBeActivated();
			if(bBlock){
				++m_slots[i].m_blockCounter;
				VERIFY2(m_slots[i].m_blockCounter< 5,"block slots overflow");
			}else{
				--m_slots[i].m_blockCounter;
				VERIFY2(m_slots[i].m_blockCounter>-5,"block slots underflow");
			}
			if(bCanBeActivated != m_slots[i].CanBeActivated())
				bChanged = true;
		}
	}
	if(bChanged)
	{
		u32 ActiveSlot		= GetActiveSlot();
		u32 PrevActiveSlot	= GetPrevActiveSlot();
		if(ActiveSlot==NO_ACTIVE_SLOT)
		{//try to restore hidden weapon
			if(PrevActiveSlot!=NO_ACTIVE_SLOT && m_slots[PrevActiveSlot].CanBeActivated()) 
				if(Activate(PrevActiveSlot))
					SetPrevActiveSlot(NO_ACTIVE_SLOT);
		}else
		{//try to hide active weapon
			if(!m_slots[ActiveSlot].CanBeActivated() )
				if(Activate(NO_ACTIVE_SLOT))
					SetPrevActiveSlot(ActiveSlot);
		}
	}
}
