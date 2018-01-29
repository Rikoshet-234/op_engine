#include "pch_script.h"
#include "ExoOutfit.h"
#include "ai_space.h"
#include "script_engine.h"


bool script_is_charged(CExoOutfit *exo)
{
	if (!exo)
	{
		Msg("! ERROR CExoOutfit::script_is_charged invlid input object");
		return false;
	}
	return exo->isBatteryPresent();
}

luabind::object script_discharge(CExoOutfit *exo, bool spawn = false)
{
	luabind::object lua_table = luabind::newtable(ai().script_engine().lua());
	if (!exo)
	{
		Msg("! ERROR CExoOutfit::script_discharge invlid input object");
		return lua_table;
	}
	lua_table["section"] = exo->m_sCurrentBattery.c_str();
	lua_table["charge"]= exo->m_fCurrentCharge;
	exo->RemoveFromBatterySlot(spawn);
	return lua_table;
}

void CExoOutfit::script_register(lua_State *L)
{
	luabind::module(L)
		[
			luabind::class_<CExoOutfit>("CExoOutfit")
			.def("is_charged", &script_is_charged)
			.def("discharge", &script_discharge)
			//.def("charge", &CExoOutfit::PutToBatterySlot)
		];
}