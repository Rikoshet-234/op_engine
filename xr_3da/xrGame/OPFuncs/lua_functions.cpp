#include "stdafx.h"
#include "lua_functions.h"

#include <luabind/luabind.hpp>
#include "../xr_3da/xrGame/inventory_space.h"
#include "../xrCore/OPFuncs/op_engine_version.h"
#include "../ai_space.h"
#include "../script_engine.h"
#include "../script_game_object.h"
#include "../ui/UIInventoryUtilities.h"


namespace OPFuncs
{
	bool luaFunctionExist(lua_State *L,std::string functionPath)
	{
		int save_top = lua_gettop(L);
		std::string name_space = functionPath.substr(0,functionPath.find("."));
		std::string func_name  = functionPath.substr(name_space.length());
		if (!func_name.empty())
		{
			lua_getglobal(L, name_space.c_str());
			if (lua_istable(L, -1))
				lua_getfield(L, -1, func_name.c_str());
			else
			{
				lua_settop(L, save_top);
				return false;
			}
		}
		else
		{
			func_name = name_space;
			name_space = "_G";
			lua_getglobal(L, func_name.c_str()); 
		}
		return !lua_isfunction(L, -1);

	}

	u32	get_actor_slots_count()
	{
		return SLOTS_TOTAL-1; //from inventory_space.h
	}	

	LPCSTR lua_GetOPEngineVersion()
	{
		std::string version=GetOPEngineVersion();
		char* result=new char[version.length()+1];
		strcpy(result,version.c_str());
		return result;
	}

	bool lua_IsOPEngine()
	{
		return true;

	}


	void registerLuaOPFuncs(lua_State *L)
	{
		luabind::module(L)
			[
				luabind::def("get_actor_slots_count",&get_actor_slots_count),
				luabind::def("IsOPEngine", &lua_IsOPEngine),
				luabind::def("engine_log", static_cast<void(*)(LPCSTR)>(&Log)),
				luabind::def("GetOPEngineVersion",&lua_GetOPEngineVersion),
				luabind::def("get_weapon_vstatic",&GetWeaponStatic)
			];
	}

	CUIStatic* GetWeaponStatic(CWeapon weapon)
	{
		UIIconInfo iconInfo(weapon.cNameSect().c_str());
		CUIStatic *weaponStatic = xr_new<CUIStatic>();
		weaponStatic->SetAutoDelete(true);
		weaponStatic->SetStretchTexture	(true);
		weaponStatic->SetShader(InventoryUtilities::GetEquipmentIconsShader());
		weaponStatic->SetColor(color_rgba(255,255,255,192));
		weaponStatic->GetUIStaticItem().SetShader(InventoryUtilities::GetEquipmentIconsShader());
		weaponStatic->GetUIStaticItem().SetOriginalRect(iconInfo.getOriginalRect());
		weaponStatic->ClipperOn();
		return weaponStatic;


		/*
		auto cell_item = xr_new<CUIWeaponCellItem>(wpn);
		if (wpn->SilencerAttachable() && wpn->IsSilencerAttached() )
		{
			auto sil = init_addon(cell_item, *wpn->GetSilencerName(), scale, scale_x, eAddonType::eSilencer);
			UIPickUpItemIcon.AttachChild(sil);
		}
		if (wpn->ScopeAttachable() && wpn->IsScopeAttached() )
		{
			auto scope = init_addon(cell_item, *wpn->GetScopeName(), scale, scale_x, eAddonType::eScope);
			UIPickUpItemIcon.AttachChild(scope);
		}
		if (wpn->GrenadeLauncherAttachable() && wpn->IsGrenadeLauncherAttached() )
		{
			auto launcher = init_addon(cell_item, *wpn->GetGrenadeLauncherName(), scale, scale_x, eAddonType::eLauncher);
			UIPickUpItemIcon.AttachChild(launcher);
		}
		delete_data(cell_item);
		*/
	}
}
 