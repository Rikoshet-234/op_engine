#include "pch_script.h"
#include "script_game_object.h"
#include "alife_space.h"
#include "script_entity_space.h"
#include "movement_manager_space.h"
#include "script_callback_ex.h"
#include "memory_space.h"
#include "cover_point.h"
#include "script_hit.h"
#include "script_binder_object.h"
#include "InventoryOwner.h"
#include "InventoryBox.h"
#include "Artifact.h"
#include "HUDManager.h"
#include "ui/UIMainIngameWnd.h"
#include "action_planner.h"
#include "script_storage_space.h"
#include "script_engine.h"
#include "CustomMonster.h"
#include "CustomOutfit.h"
#include "Actor.h"
#include "Actor_Flags.h"
#include "Inventory.h"
#include "inventory_item.h"
#include "Weapon.h"
#include "ui/UICarBodyWnd.h"
#include "inventory_slots_script_enum.h"
#include "item_place_change_enum.h"
#include "ui/UIInventoryWnd.h"
#include "ui/UITalkWnd.h"
#include "ui/UITradeWnd.h"
#include "UIGameSP.h"
#include "NightVisionDevice.h"
#include "OPFuncs/utils.h"
#include "xrServer_Objects_ALife_Items.h"

using namespace luabind;

void CInventoryItem::script_register(lua_State *L)
{
	module(L)
		[
			class_<CInventoryItem>("CInventoryItem")
			.def("Name",				&CInventoryItem::Name)
			.def("GetGameObject",		&CInventoryItem::GetGameObject)
		];
}

u32 CScriptGameObject::GetSlot() const
{
	CInventoryItem		*inventory_item = smart_cast<CInventoryItem*>(&object());
	if (!inventory_item) {
		ai().script_engine().script_log			(ScriptStorage::eLuaMessageTypeError,"CSciptEntity : cannot access class member GetSlot!");
		return			(false);
	}
	return				(inventory_item->GetSlot());
}

LPCSTR CScriptGameObject::GetVisualName() const
{
	if (!g_pGameLevel)
	{
		Msg("Error! CScriptGameObject::GetVisualName : game level doesn't exist. wtf?????");
		return "";
	}
	return	*(object().cNameVisual());

}

bool CScriptGameObject::InventoryMoveItem(CScriptGameObject* item,u32 to,bool force) const
{
	if (to==NO_ACTIVE_SLOT || !item)
		return false;
	CInventoryOwner* pInventoryOwner = smart_cast<CInventoryOwner*>(&object());
	if (!pInventoryOwner)
	{
		Msg("! ERROR CScriptGameObject::InventoryMoveItem cannot acces to CInventoryOwner members!");
		return false;
	}
	CInventoryItem* itemFrom=smart_cast<CInventoryItem*>(&(item->object()));
	if (!itemFrom)
	{
		Msg("! ERROR CScriptGameObject::InventoryMoveItem cannot cast to CInventoryItem itemFrom!");
		return false;
	}
	bool moved=false;
	if (to<SLOTS_TOTAL)
	{
		CInventoryItem* itemTo=pInventoryOwner->inventory().ItemFromSlot(to);
		if (itemTo)
		{
			if (itemTo->object().lua_game_object()==item)
			{
				Msg("~ WARNING CScriptGameObject::InventoryMoveItem cannot move himself");
				return false;
			}
			if (!force)
			{
				Msg("~ WARNING CScriptGameObject::InventoryMoveItem destination slot is busy!");
				return false;
			}
			moved=pInventoryOwner->inventory().Ruck(itemTo);
			if (!moved)
			{
				Msg("~ ERROR CScriptGameObject::InventoryMoveItem cant put item from slot to ruck!");
				return false;
			}
		}
		moved=pInventoryOwner->inventory().Slot(itemFrom,false);
		if (!moved)
		{
			Msg("~ WARNING CScriptGameObject::InventoryMoveItem cannot put item to slot!");
			return false;
		}
	}
	else if (to==InventorySlots::RUCK || to==InventorySlots::BELT)
	{
		switch (to)
		{
			case InventorySlots::RUCK:
				{
					moved=pInventoryOwner->inventory().Ruck(itemFrom);
					if (!moved)
					{
						Msg("~ WARNING CScriptGameObject::InventoryMoveItem cannot put item [%s] to ruck!",itemFrom->object().cName().c_str());
						return false;
					}
				}
				break;
			case InventorySlots::BELT:
				{
					moved=pInventoryOwner->inventory().Belt(itemFrom);
					if (!moved)
					{
						Msg("~ WARNING CScriptGameObject::InventoryMoveItem cannot put item [%s] to belt!",itemFrom->object().cName().c_str());
						return false;
					}
				}
				break;
			default:
				break;
		}
	}
	else
	{
		Msg("! ERROR CScriptGameObject::InventoryMoveItem invalid destination!");
		return false;
	}
	if (moved)
	{
		CUIGameSP* pGameSP = smart_cast<CUIGameSP*>(HUD().GetUI()->UIGame());
		pGameSP->ReInitShownUI();
	}
	return true;
}

#pragma region invulnerabilities from scripts
bool CScriptGameObject::invulnerable		() const
{
	CCustomMonster	*monster = smart_cast<CCustomMonster*>(&object());
	if (!monster) {
		CActor* actor=smart_cast<CActor*>(&object());
		if (actor)
		{
			return actor_invulnerable();
		}
		else
		{
			ai().script_engine().script_log			(ScriptStorage::eLuaMessageTypeError,"CCustomMonster : cannot access class member invulnerable!");
			return		(false);
		}
	}

	return			(monster->invulnerable());
}

void CScriptGameObject::invulnerable		(bool invulnerable)
{
	CCustomMonster	*monster = smart_cast<CCustomMonster*>(&object());
	if (!monster) {
		CActor* actor=smart_cast<CActor*>(&object());
		if (actor)
		{
			actor_invulnerable(invulnerable);
			return;
		}
		else
		{
			ai().script_engine().script_log			(ScriptStorage::eLuaMessageTypeError,"CCustomMonster : cannot access class member invulnerable!");
			return;
		}
	}

	monster->invulnerable	(invulnerable);
}

bool				CScriptGameObject::actor_invulnerable						() const
{
	return !!psActorFlags.test(AF_GODMODE_PARTIAL);
}

void				CScriptGameObject::actor_invulnerable						(bool invulnerable)
{
	psActorFlags.set(AF_GODMODE_PARTIAL,invulnerable);
}
#pragma endregion

#pragma region weight functions

float CScriptGameObject::GetActorMaxWeight() const
{
	CActor* pActor = smart_cast<CActor*>(&object());
	if(!pActor) {
		ai().script_engine().script_log			(ScriptStorage::eLuaMessageTypeError,"CActor : cannot access class member GetActorMaxWeight!");
		return			(false);
	}
	return				(pActor->inventory().GetMaxWeight());
}

float CScriptGameObject::GetTotalWeight() const
{
	CInventoryOwner	*inventory_owner = smart_cast<CInventoryOwner*>(&object());
	if (!inventory_owner) {
		ai().script_engine().script_log			(ScriptStorage::eLuaMessageTypeError,"CInventoryOwner : cannot access class member GetTotalWeight!");
		return			(false);
	}
	return				(inventory_owner->inventory().TotalWeight());
}

float CScriptGameObject::Weight() const
{
	CInventoryItem		*inventory_item = smart_cast<CInventoryItem*>(&object());
	if (!inventory_item) {
		ai().script_engine().script_log			(ScriptStorage::eLuaMessageTypeError,"CSciptEntity : cannot access class member Weight!");
		return			(false);
	}
	return				(inventory_item->Weight());
}
#pragma endregion

#pragma region crouch functions
void CScriptGameObject::actor_set_crouch() const
{
	CActor* actor= smart_cast<CActor*>(&this->object());
	if (!actor)
	{
		ai().script_engine().script_log		(ScriptStorage::eLuaMessageTypeError,"CActor::set_crouch non-Actor object !!!");
		return;
	}
	g_bAutoClearCrouch=false;
	actor->character_physics_support()->movement()->EnableCharacter();
	actor->character_physics_support()->movement()->ActivateBoxDynamic(1);
	actor->set_state(actor->get_state() | mcCrouch);
	actor->set_state_wishful(actor->get_state_wishful() | mcCrouch);
	HUD().GetUI()->UIMainIngameWnd->MotionIcon().ShowState(CUIMotionIcon::stCrouch);
}

bool CScriptGameObject::actor_is_crouch() const
{
	CActor* actor= smart_cast<CActor*>(&this->object());
	if (!actor)
	{
		ai().script_engine().script_log		(ScriptStorage::eLuaMessageTypeError,"CActor::is_crouch non-Actor object !!!");
		return false;
	}
	return !!(actor->get_state()&mcCrouch);
}
#pragma endregion

#pragma region artefact immunities

luabind::object CScriptGameObject::GetImmunitiesTable() const
{
	luabind::object immunities = luabind::newtable(ai().script_engine().lua());
	CArtefact* artefact=smart_cast<CArtefact*>(&this->object());
	if (!artefact)
	{
		Msg("! ERROR CScriptGameObject::GetImmunitiesTable cannot cast object [%s] to CArtefact",this->cName().c_str());
		return immunities;
	}
	HitImmunity::HitTypeSVec hitVector=artefact->m_ArtefactHitImmunities.GetHitTypeVec();
	for (HitImmunity::HitTypeSVec::iterator it=hitVector.begin();it!=hitVector.end();++it)
		if (*it!=1.0f)
			immunities[std::distance(hitVector.begin(),it)]=1.0f-*it;
	return immunities;
}

luabind::object	CScriptGameObject::GetImmunitiesFromBeltTable() const
{
	CInventoryOwner	*inventory_owner = smart_cast<CInventoryOwner*>(&object());
	if (!inventory_owner) {
		return		luabind::object();
	}
	xr_vector<CInventoryItem* >::const_iterator bi=inventory_owner->inventory().m_belt.begin();
	xr_vector<CInventoryItem* >::const_iterator ei=inventory_owner->inventory().m_belt.end();
	xr_map<int,float> data;
	std::for_each(bi,ei,[&](CInventoryItem* item)
	{
		CArtefact* artefact=smart_cast<CArtefact*>(item);
		if (artefact)
		{
			HitImmunity::HitTypeSVec hitVector=artefact->m_ArtefactHitImmunities.GetHitTypeVec();
			for (HitImmunity::HitTypeSVec::iterator it=hitVector.begin();it!=hitVector.end();++it)
				if (*it!=1.0f)
				{
					int immIndex=std::distance(hitVector.begin(),it);
					xr_map<int,float>::iterator current=data.find(immIndex);
					float immValue=1.0f-*it; 
					if (current!=data.end())
						current->second+=immValue;
					else
						data.insert(mk_pair<int,float>(immIndex,immValue));
				}
		}
	});
	luabind::object result_l = luabind::newtable(ai().script_engine().lua());
	if (data.size()>0)
		std::for_each(data.begin(),data.end(),[&](std::pair<int,float> it)
		{
			result_l[it.first]=it.second;
		});
	return result_l;
}

#pragma endregion

#pragma region get main precreated  windows
CUIInventoryWnd* get_inventory_wnd()
{
	CUIGameSP* pGameSP = smart_cast<CUIGameSP*>(HUD().GetUI()->UIGame());
	return pGameSP->InventoryMenu;
}

CUICarBodyWnd* get_carbody_wnd()
{
	CUIGameSP* pGameSP = smart_cast<CUIGameSP*>(HUD().GetUI()->UIGame());
	return pGameSP->UICarBodyMenu;
}

CUITradeWnd* get_trade_wnd()
{
	CUIGameSP* pGameSP = smart_cast<CUIGameSP*>(HUD().GetUI()->UIGame());
	return pGameSP->TalkMenu->GetTradeWnd();
}
#pragma endregion

#pragma region add functionality for callback,returning bool value

void CScriptGameObject::SetCallbackEx(GameObject::ECallbackType type, const luabind::functor<bool> &functor)
{
	object().callback_ex(type).set(functor);
}

void CScriptGameObject::SetCallbackEx(GameObject::ECallbackType type, const luabind::functor<bool> &functor, const luabind::object &object)
{
	this->object().callback_ex(type).set(functor, object);
}

void CScriptGameObject::SetCallbackEx(GameObject::ECallbackType type)
{
	object().callback_ex(type).clear();
}

#pragma endregion

#pragma region iterate items in boxes
void CScriptGameObject::IterateInventoryBoxObject(luabind::functor<void> functor,bool showError) const
{
	CInventoryBox			*inventoryBox = smart_cast<CInventoryBox*>(&this->object());
	if (!inventoryBox) {
		ai().script_engine().script_log		(ScriptStorage::eLuaMessageTypeError,"CScriptGameObject::IterateInventoryBoxOnlyFunctor non-CInventoryBox object !!!");
		return;
	}

	xr_vector<u16>::iterator	I_id = inventoryBox->GetItemsVector()->begin();
	xr_vector<u16>::iterator	E = inventoryBox->GetItemsVector()->end();
	for ( ; I_id != E; ++I_id)
	{
		CInventoryItem* item=smart_cast<PIItem>(Level().Objects.net_Find(*I_id));
		if (item)
			functor				(item->object().lua_game_object());
		else
			if (showError)
				Msg("! ERROR invalid id [%i] in inventoryBox. Not found in object registry!",*I_id);
	}
}

void CScriptGameObject::IterateInventoryBoxId(luabind::functor<void> functor) const
{
	CInventoryBox			*inventoryBox = smart_cast<CInventoryBox*>(&this->object());
	if (!inventoryBox) {
		ai().script_engine().script_log		(ScriptStorage::eLuaMessageTypeError,"CScriptGameObject::IterateInventoryBoxOnlyFunctor non-CInventoryBox object !!!");
		return;
	}
	xr_vector<u16>::iterator	I_id = inventoryBox->GetItemsVector()->begin();
	xr_vector<u16>::iterator	E = inventoryBox->GetItemsVector()->end();
	for ( ; I_id != E; ++I_id)
		functor				(*I_id);
}

#pragma endregion

CNightVisionDevice* CScriptGameObject::GetCurrentPNV()
{
	CActor* pActor = smart_cast<CActor*>(&object());	
	if(!pActor) 
		return nullptr;
	CCustomOutfit* outfit = smart_cast<CCustomOutfit*>(pActor->inventory().ItemFromSlot(OUTFIT_SLOT));
	if (outfit && outfit->GetNightVisionDevice())
		return outfit->GetNightVisionDevice();
	CNightVisionDevice* nvd = smart_cast<CNightVisionDevice*>(pActor->inventory().ItemFromSlot(PNV_SLOT));
	return nvd;
}

bool CScriptGameObject::get_useful_for_npc() const
{
	CInventoryItem	*item = smart_cast<CInventoryItem*>(&object());
	if (!item)
	{
		ai().script_engine().script_log(ScriptStorage::eLuaMessageTypeError, "CScriptGameObject::get_useful_for_npc non-CInventoryItem object !!!");
		return false;
	}
	return item->useful_for_NPC();
}

#pragma region weapon manipulation
bool CScriptGameObject::has_scope() const
{
	CWeapon		*weapon = smart_cast<CWeapon*>(&object());
	if (!weapon) {
		ai().script_engine().script_log(ScriptStorage::eLuaMessageTypeError, "call [has_scope] for non-CWeapon object !!!");
		return false;
	}
	return weapon->IsScopeAttached();
}
bool CScriptGameObject::has_silencer() const
{
	CWeapon		*weapon = smart_cast<CWeapon*>(&object());
	if (!weapon) {
		ai().script_engine().script_log(ScriptStorage::eLuaMessageTypeError, "call [has_silencer] for non-CWeapon object !!!");
		return false;
	}
	return weapon->IsSilencerAttached();
}
bool CScriptGameObject::has_grenadelauncher() const
{
	CWeapon		*weapon = smart_cast<CWeapon*>(&object());
	if (!weapon) {
		ai().script_engine().script_log(ScriptStorage::eLuaMessageTypeError, "call [has_grenadelauncher] for non-CWeapon object !!!");
		return false;
	}
	return weapon->IsGrenadeLauncherAttached();
}

LPCSTR detach_local(CGameObject* object, CSE_ALifeItemWeapon::EWeaponAddonState addonType, luabind::object const &param)
{
	CWeapon		*weapon = smart_cast<CWeapon*>(object);
	if (!weapon) {
		ai().script_engine().script_log(ScriptStorage::eLuaMessageTypeError, "call [detach_local] for non-CWeapon object!");
		return nullptr;
	}
	if (!param.is_valid())
	{
		ai().script_engine().script_log(ScriptStorage::eLuaMessageTypeError, "invalid input param [force_spawn] for [detach_local]!");
		return nullptr;
	}
	bool force_spawn = true;
	if (param.type() == LUA_TBOOLEAN)
		force_spawn = luabind::object_cast<bool>(param);
	shared_str addon_name;
	bool allowDetach = false;
	switch (addonType)
	{
		case CSE_ALifeItemWeapon::eWeaponAddonScope: 
			addon_name = weapon->GetScopeName().c_str();
			allowDetach = weapon->ScopeAttachable() && weapon->IsScopeAttached();
			break;
		case CSE_ALifeItemWeapon::eWeaponAddonGrenadeLauncher: 
			addon_name = weapon->GetGrenadeLauncherName().c_str();
			allowDetach = weapon->GrenadeLauncherAttachable() && weapon->IsGrenadeLauncherAttached();
			break;
		case CSE_ALifeItemWeapon::eWeaponAddonSilencer: 
			{
				addon_name = weapon->GetSilencerName().c_str();
				allowDetach = weapon->SilencerAttachable() && weapon->IsSilencerAttached();
			}
			break;
		default: ;
	}
	if (allowDetach)
		OPFuncs::DetachAddon(weapon, addon_name.c_str(), force_spawn);
	return  addon_name.size()>0 ? addon_name.c_str() : nullptr;
}

LPCSTR CScriptGameObject::detach_scope(luabind::object const &param)
{
	return detach_local(&object(), CSE_ALifeItemWeapon::EWeaponAddonState::eWeaponAddonScope, param);
}

LPCSTR CScriptGameObject::detach_silencer(luabind::object const &param)
{
	return detach_local(&object(), CSE_ALifeItemWeapon::EWeaponAddonState::eWeaponAddonSilencer, param);
}

LPCSTR CScriptGameObject::detach_grenadelauncher(luabind::object const &param)
{
	return detach_local(&object(), CSE_ALifeItemWeapon::EWeaponAddonState::eWeaponAddonGrenadeLauncher, param);
}

LPCSTR CScriptGameObject::GetCurrentAmmoSection()
{
	CWeapon		*weapon = smart_cast<CWeapon*>(&object());
	if (weapon) 
		return weapon->m_ammoTypes[weapon->m_ammoType].c_str();
	ai().script_engine().script_log(ScriptStorage::eLuaMessageTypeError, "call [current_ammo_section] for non-CWeapon object!");
	return nullptr;
}

void CScriptGameObject::FullUnloadWeapon()
{
	CWeapon		*weapon = smart_cast<CWeapon*>(&object());
	if (weapon)
	{
		CWeaponMagazined		*weaponM = smart_cast<CWeaponMagazined*>(weapon);
		if (weaponM) {
			OPFuncs::UnloadWeapon(weaponM);
			return;
		}
	}
	ai().script_engine().script_log(ScriptStorage::eLuaMessageTypeError, "call [full_unload_weapon] for non-CWeaponMagazined object!");
}
#pragma endregion

class_<CScriptGameObject> &script_register_game_object3(class_<CScriptGameObject> &instance)
{
	instance
		.def("get_slot", &CScriptGameObject::GetSlot)
		.def("get_inventory_wnd", &get_inventory_wnd)
		.def("get_carbody_wnd", &get_carbody_wnd)
		.def("get_trade_wnd", &get_trade_wnd)
		.def("invulnerable", static_cast<bool (CScriptGameObject::*)		() const>	(&CScriptGameObject::invulnerable))
		.def("invulnerable", static_cast<void (CScriptGameObject::*)		(bool)>		(&CScriptGameObject::invulnerable))
		.def("max_weight", &CScriptGameObject::GetActorMaxWeight)
		.def("total_weight", &CScriptGameObject::GetTotalWeight)
		.def("item_weight", &CScriptGameObject::Weight)
		.def("is_crouch", &CScriptGameObject::actor_is_crouch)
		.def("set_crouch", &CScriptGameObject::actor_set_crouch)
		.def("get_visual_name", &CScriptGameObject::GetVisualName)
		.def("get_immunities_from_belt", &CScriptGameObject::GetImmunitiesFromBeltTable)
		.def("get_immunities", &CScriptGameObject::GetImmunitiesTable)

		.def("set_callback_ex", static_cast<void (CScriptGameObject::*)		(GameObject::ECallbackType, const luabind::functor<bool>&)>								(&CScriptGameObject::SetCallbackEx))
		.def("set_callback_ex", static_cast<void (CScriptGameObject::*)		(GameObject::ECallbackType, const luabind::functor<bool>&, const luabind::object&)>		(&CScriptGameObject::SetCallbackEx))
		.def("set_callback_ex", static_cast<void (CScriptGameObject::*)		(GameObject::ECallbackType)>															(&CScriptGameObject::SetCallbackEx))

		.def("inventory_move_item", &CScriptGameObject::InventoryMoveItem)

		.def("is_game_object", &CScriptGameObject::IsGameObject)
		.def("is_car", &CScriptGameObject::IsCar)
		.def("is_helicopter", &CScriptGameObject::IsHeli)
		.def("is_entity_alive", &CScriptGameObject::IsEntityAlive)
		.def("is_artefact", &CScriptGameObject::IsArtefact)
		.def("is_actor", &CScriptGameObject::IsActor)
		.def("is_weapon", &CScriptGameObject::IsWeapon)
		.def("is_medkit", &CScriptGameObject::IsMedkit)
		.def("is_eatable", &CScriptGameObject::IsEatableItem)
		.def("is_antirad", &CScriptGameObject::IsAntirad)
		.def("is_outfit", &CScriptGameObject::IsCustomOutfit)
		.def("is_scope", &CScriptGameObject::IsScope)
		.def("is_silencer", &CScriptGameObject::IsSilencer)
		.def("is_grenade_launcher", &CScriptGameObject::IsGrenadeLauncher)
		.def("is_weapon_magazined", &CScriptGameObject::IsWeaponMagazined)
		.def("is_stalker", &CScriptGameObject::IsStalker)
		.def("is_monster", &CScriptGameObject::IsMonster)
		.def("is_trader", &CScriptGameObject::IsTrader)
		.def("is_ammo", &CScriptGameObject::IsAmmo)
		.def("is_missile", &CScriptGameObject::IsMissile)
		.def("is_grenade", &CScriptGameObject::IsGrenade)
		.def("is_bottle", &CScriptGameObject::IsBottleItem)
		.def("is_torch", &CScriptGameObject::IsTorch)
		.def("is_weaponGL", &CScriptGameObject::IsWeaponGL)
		.def("is_inventory_box", &CScriptGameObject::IsInventoryBox)
		.def("is_pnv", &CScriptGameObject::IsPNV)
		.def("iterate_box_obj", &CScriptGameObject::IterateInventoryBoxObject)
		.def("iterate_box_obj", &CScriptGameObject::IterateInventoryBoxId)
		.def("get_current_pnv", &CScriptGameObject::GetCurrentPNV)
		.def("useful_for_npc", &CScriptGameObject::get_useful_for_npc)

		.def("has_scope", &CScriptGameObject::has_scope)
		.def("has_silencer", &CScriptGameObject::has_silencer)
		.def("has_grenadelauncher", &CScriptGameObject::has_grenadelauncher)

		.def("full_unload_weapon", &CScriptGameObject::FullUnloadWeapon)
		.def("current_ammo_section", &CScriptGameObject::GetCurrentAmmoSection)
		.def("detach_scope", &CScriptGameObject::detach_scope)
		.def("detach_silencer", &CScriptGameObject::detach_silencer)
		.def("detach_grenadelauncher", &CScriptGameObject::detach_grenadelauncher)
		;

	return	(instance);
}
