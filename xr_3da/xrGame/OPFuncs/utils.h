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
	LPCSTR boolToStr(bool value);
	std::string GetMonsterInfoStr(CAI_Stalker);
	void splitString(const std::string &s, char delim, std::vector<std::string> &elems);
	std::vector<std::string> splitString(const std::string &s, char delim);
	double round(double number);

	xr_vector<LPCSTR> getStringsFromLua(luabind::object const& table);
	std::string getComplexString(std::string untranslatedString,PIItem item,std::string untranslatedString2="",std::string untranslatedString3="");
	std::string getAddonInvName(std::string addonName);
	void UnloadWeapon(CWeaponMagazined* weapon);
	void DetachAddon(CInventoryItem* item,const char* addon_name);
	void AttachAddon(CInventoryItem* item_to_upgrade,CInventoryItem* addon);
	bool IsUsedInInventory(CInventoryOwner* owner,CInventoryItem* pItem);
	//check string is contains only 'english' symbols,numbers and '_'
	bool isCorrectString(std::string str);
	//create map to combine hit type and descriprion for him
	xr_map<ALife::EHitType,shared_str> CreateImmunesStringMap();

	struct restoreParam
	{
		shared_str paramName;
		shared_str paramDesc;
		restoreParam(){}
		restoreParam(shared_str name,shared_str desc):paramName(name),paramDesc(desc){}
	};

	//create map to combine restore type,descriptions and quick_access index
#define BLEEDING_RESTORE_ID 0
#define SATIETY_RESTORE_ID 1
#define RADIATION_RESTORE_ID 2
#define HEALTH_RESTORE_ID 3
#define POWER_RESTORE_ID 4
#define POWER_LOSS_ID 5
	xr_map<int,restoreParam> CreateRestoresStringMap();
}

#endif
