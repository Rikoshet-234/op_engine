#include "pch_script.h"

#include "utils.h"
#include "lua.h"
#include "../string_table.h"
#include "../../../xrCore/xr_ini.h"
#include "../../defines.h"
#include "../inventory_item.h"


CTimerStat forCellCreation; 
CTimerStat forFillActor; 
CTimerStat forFillOther; 

namespace OPFuncs
{
	
	double round(double number)
	{
		return number < 0.0 ? ceil(number - 0.5) : floor(number + 0.5);
	}

	std::string getAddonInvName(std::string addonName)
	{
		std::string name;
		if (addonName.size()>0)
		{
			if (pSettings->line_exist(addonName.c_str(),"inv_name_short"))
				name=pSettings->r_string(addonName.c_str(),"inv_name_short");
			if ((name.size()==0) || (std::strcmp(name.c_str(),EMPTY_DESC)==0))
				name=pSettings->r_string(addonName.c_str(),"inv_name");
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

	std::string getComplexString(std::string untranslatedString,PIItem item,std::string untranslatedString2)
	{
		std::string translateString=*CStringTable().translate(untranslatedString.c_str());
		if (g_uCommonFlags.test(E_COMMON_FLAGS::uiShowExtDesc))
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

	xr_vector<LPCSTR> getStringsFromLua(luabind::object const& table) 
	{
		xr_vector<LPCSTR> luaStrings;
		if (table!=nullptr && table.is_valid() && table.type()==LUA_TTABLE)
			for(luabind::object::iterator iter=table.begin(); iter != table.end(); ++iter)
			{
				switch(iter->type())
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
		std::string usName=monster.Name();
		std::string secName=monster.cNameSect().c_str();
		std::string objName=monster.Name_script();
		string2048 strFmr;
		sprintf_s(strFmr,"ObjName [%s] SectionName [%s] IngameName [%s]",objName,secName,usName);
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
}