#include "pch_script.h"
#include "inventory_slots_script_enum.h"

using namespace luabind;

void CInventorySlots::script_register(lua_State *L)
{
	module(L)
	[
		class_<enum_exporter<InventorySlots::InventorySlotsEnum> >("inventory_slots")
			.enum_("inventory_slots")
			[
				value("NO_ACT_SLOT",	int(NO_ACTIVE_SLOT)),
				value("KNIFE",			int(KNIFE_SLOT)),
				value("PISTOL",			int(PISTOL_SLOT)),
				value("RIFLE",			int(RIFLE_SLOT)),
				value("GRENADE",		int(GRENADE_SLOT)),
				value("APPARATUS",		int(APPARATUS_SLOT)),
				value("BOLT",			int(BOLT_SLOT)),
				value("OUTFIT",			int(OUTFIT_SLOT)),
				value("PDA",			int(PDA_SLOT)),
				value("DETECTOR_ARTS",	int(DETECTOR_ARTS_SLOT)),
				value("DETECTOR_ANOMS",	int(DETECTOR_ANOM_SLOT)),
				value("TORCH",			int(TORCH_SLOT)),
				value("ARTEFACT",		int(ARTEFACT_SLOT)),
				value("PNV",			int(PNV_SLOT)),
				value("SHOTGUN",		int(SHOTGUN_SLOT)),
				value("BIODEV",			int(BIODEV_SLOT)),
				value("RUCK",			int(InventorySlots::RUCK)),
				value("BELT",			int(InventorySlots::BELT))
			]
	];
};