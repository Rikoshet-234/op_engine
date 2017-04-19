#include "../pch_script.h"

#include "utils.h"
#include "lua.h"
#include "../string_table.h"
#include "../../../xrCore/xr_ini.h"
#include "../../defines.h"
#include "../inventory_item.h"
#include "../../xrSound/Sound.h"
#include "../ui/xrUIXmlParser.h"
#include "../Level.h"
#include "../Actor.h"
#include "../../xrNetServer/NET_utils.h"
#include "../Inventory.h"
#include "../script_engine.h"
#include "../ai_space.h"

#include <locale>
#include <iostream>

CTimerStat forCellCreation; 
CTimerStat forFillActor; 
CTimerStat forFillOther; 

namespace OPFuncs
{
	void replaceAll(std::string& str, const std::string& from, const std::string& to)
	{
		if (from.empty())
			return;
		size_t start_pos = 0;
		while ((start_pos = str.find(from, start_pos)) != std::string::npos) {
			str.replace(start_pos, from.length(), to);
			start_pos += to.length();
		}
	}

	LPCSTR boolToStr(bool value)
	{
		return value ? "true" : "false";
	}
	double round(double number)
	{
		return number < 0.0 ? ceil(number - 0.5) : floor(number + 0.5);
	}

	std::string getAddonInvName(std::string addonName)
	{
		std::string name;
		if (addonName.size() > 0)
		{
			if (pSettings->line_exist(addonName.c_str(), "inv_name_short"))
				name = pSettings->r_string(addonName.c_str(), "inv_name_short");
			if ((name.size() == 0) || (xr_strcmp(name.c_str(), EMPTY_DESC) == 0))
				name = pSettings->r_string(addonName.c_str(), "inv_name");
		}
		return name;
	}

	void UnloadWeapon(CWeaponMagazined* weapon)
	{
		weapon->UnloadMagazine();
		CWeaponMagazinedWGrenade* weaponWithGrenade = smart_cast<CWeaponMagazinedWGrenade*>(weapon);
		if (weaponWithGrenade) //unload other ammos
		{
			weaponWithGrenade->PerformSwitchGL();
			weaponWithGrenade->UnloadMagazine();
			weaponWithGrenade->PerformSwitchGL();
		}
	}

	void DetachAddon(CInventoryItem* item, const char* addon_name)
	{
		if (OnClient())
		{
			NET_Packet								P;
			item->object().u_EventGen(P, GE_ADDON_DETACH, item->object().ID());
			P.w_stringZ(addon_name);
			item->object().u_EventSend(P);
		};
		item->Detach(addon_name, true);

		CActor *pActor = smart_cast<CActor*>(Level().CurrentEntity());
		if (pActor && item == pActor->inventory().ActiveItem())
		{
			pActor->inventory().Activate(NO_ACTIVE_SLOT);
		}
	}

	void AttachAddon(CInventoryItem* item_to_upgrade, CInventoryItem* addon)
	{
		if (OnClient())
		{
			NET_Packet								P;
			item_to_upgrade->object().u_EventGen(P, GE_ADDON_ATTACH, item_to_upgrade->object().ID());
			P.w_u32(addon->object().ID());
			item_to_upgrade->object().u_EventSend(P);
		};
		item_to_upgrade->Attach(addon, true);
		//спрятать вещь из активного слота в инвентарь на время вызова менюшки
		CActor *pActor = smart_cast<CActor*>(Level().CurrentEntity());
		if (pActor && item_to_upgrade == pActor->inventory().ActiveItem())
			pActor->inventory().Activate(NO_ACTIVE_SLOT);
	}

	bool IsUsedInInventory(CInventoryOwner* owner, CInventoryItem* pItem)
	{
		TIItemContainer belt = owner->inventory().m_belt;
		TIItemContainer::iterator it = belt.begin();
		TIItemContainer::iterator it_e = belt.end();
		bool found = false;
		for (; it != it_e; ++it)
		{
			if ((*it)->object().ID() == pItem->object().ID())
			{
				found = true;
				break;
			}
		}
		if (found)
			return true;
		if (owner->inventory().ItemFromSlot(OUTFIT_SLOT) == pItem)
			return true;
		if (pItem->GetSlot() != NO_ACTIVE_SLOT && owner->inventory().ItemFromSlot(pItem->GetSlot()) == pItem)
			return true;
		return false;
	}

	std::string getComplexString(std::string untranslatedString, PIItem item, std::string untranslatedString2, std::string untranslatedString3)
	{
		std::string translateString = *CStringTable().translate(untranslatedString.c_str());
		if (g_uCommonFlags.test(E_COMMON_FLAGS::uiShowExtDesc))
		{
			std::string additionString;
			if (item != nullptr)
			{
				additionString = item->m_name.c_str();
				if ((item->m_nameShort != nullptr) && (item->m_nameShort != "") && (item->m_nameShort != EMPTY_DESC))
					additionString = item->m_nameShort.c_str();
			}
			else
				additionString = *CStringTable().translate(untranslatedString2.c_str());
			return translateString + " " + additionString;
		}
		else
			if (untranslatedString3.size() > 0)
				return *CStringTable().translate(untranslatedString3.c_str());
			else
				return translateString;
	}

	xr_vector<LPCSTR> getStringsFromLua(luabind::object const& table)
	{
		xr_vector<LPCSTR> luaStrings;
		if (table != nullptr && table.is_valid() && table.type() == LUA_TTABLE)
			for (luabind::object::iterator iter = table.begin(); iter != table.end(); ++iter)
			{
				switch (iter->type())
				{
				case LUA_TSTRING:
					luaStrings.push_back(luabind::object_cast<LPCSTR>(*iter));
					break;
				default: break;
				}
			}
		return luaStrings;
	}

	std::string GetMonsterInfoStr(CAI_Stalker monster)
	{
		std::string usName = monster.Name();
		std::string secName = monster.cNameSect().c_str();
		std::string objName = monster.Name_script();
		string2048 strFmr;
		sprintf_s(strFmr, "ObjName [%s] SectionName [%s] IngameName [%s]", objName.c_str(), secName.c_str(), usName.c_str());
		return strFmr;
	}

	void splitString(const std::string &s, char delim, std::vector<std::string> &elems)
	{
		std::stringstream ss;
		ss.str(s);
		std::string item;
		while (getline(ss, item, delim)) {
			elems.push_back(item);
		}
	}

	std::vector<std::string> splitString(const std::string &s, char delim)
	{
		std::vector<std::string> elems;
		splitString(s, delim, elems);
		return elems;
	}

	bool isCorrectString(std::string str)
	{
		std::locale loc("C");
		bool contains_alpha = std::find_if(str.begin(), str.end(), [&](char c) {return std::isalpha(c, loc); }) != str.end();
		bool contains_digits = std::find_if(str.begin(), str.end(), [&](char c) {return std::isdigit(c, loc); }) != str.end();
		bool contains_punct = std::find_if(str.begin(), str.end(), [&](char c) {return c == '_'; }) != str.end();
		bool contains_other = std::find_if(str.begin(), str.end(), [&](char c)
		{
			return std::iscntrl(c, loc) || std::isspace(c, loc) || std::isblank(c, loc);
		}) != str.end();
		return (contains_alpha || contains_digits || contains_punct) && !contains_other;
	}

	std::string getFileNameFromPath(std::string fullPath)
	{
		char sep = '/';
#ifdef _WIN32
		sep = '\\';
#endif
		size_t i = fullPath.rfind(sep, fullPath.length());
		if (i != std::string::npos) {
			return(fullPath.substr(i + 1, fullPath.length() - i));
		}
		return("");
	}

	void runAlifeCallback(LPCSTR callbackName)
	{
		LPCSTR _callback = READ_IF_EXISTS(pSettings, r_string, "alife", callbackName, nullptr);
		if (!_callback || xr_strlen(_callback) == 0)
			return;
		luabind::functor<void> functor;
		bool functor_exists = ai().script_engine().functor(_callback, functor);
		if (!functor_exists)
		{
			Msg("! ERROR callback [%s] set to non exists functor [%s]", callbackName, _callback);
			return;
		}
		functor();
	}
}
