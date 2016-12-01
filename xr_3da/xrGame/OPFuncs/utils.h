#ifndef xrgUtilsH
#define xrgUtilsH

#include "../ai/stalker/ai_stalker.h"
#include "../../xrCore/FTimerStat.h"
#include "../ui/UIDragDropListEx.h"
#include "../xrCore/FTimerStat.h"
#include "../WeaponMagazinedWGrenade.h"

extern CTimerStat forCellCreation; 
extern CTimerStat forFillActor; 
extern CTimerStat forFillOther; 

#define EMPTY_DESC "UNUSED" //omg wtf

namespace OPFuncs
{
	std::string GetMonsterInfoStr(CAI_Stalker);
	void splitString(const std::string &s, char delim, std::vector<std::string> &elems);
	std::vector<std::string> splitString(const std::string &s, char delim);
	double round(double number);

	xr_vector<LPCSTR> getStringsFromLua(luabind::object const& table);
	std::string getComplexString(std::string untranslatedString,PIItem item,std::string untranslatedString2="");
	std::string getAddonInvName(std::string addonName);
	void UnloadWeapon(CWeaponMagazined* weapon);
	void DetachAddon(CInventoryItem* item,const char* addon_name);
	void AttachAddon(CInventoryItem* item_to_upgrade,CInventoryItem* addon);
}

#endif