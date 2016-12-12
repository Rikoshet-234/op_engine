#include "pch_script.h"
#include "item_place_change_enum.h"

using namespace luabind;

void CInventoryPlaceChange::script_register(lua_State *L)
{
	module(L)
	[
		class_<enum_exporter<InventoryPlaceChange::EnumItemPlaceChange> >("item_place_change")
			.enum_("item_place_change")
			[
				value("REMOVE_FROM_RUCK",				int(InventoryPlaceChange::removeFromRuck)),
				value("REMOVE_FROM_SLOT",				int(InventoryPlaceChange::removeFromSlot)),
				value("REMOVE_FROM_BELT",				int(InventoryPlaceChange::removeFromBelt)),
				value("PUT_TO_SLOT",					int(InventoryPlaceChange::putToSlot)),
				value("PUT_TO_BELT",					int(InventoryPlaceChange::putToBelt)),
				value("PUT_TO_RUCK",					int(InventoryPlaceChange::putToRuck))
			]
	];
};