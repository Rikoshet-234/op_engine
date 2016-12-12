#ifndef item_place_change_enum_h
#define item_place_change_enum_h

#include "script_export_space.h"

namespace InventoryPlaceChange
{
	enum EnumItemPlaceChange
	{
		removeFromRuck,
		removeFromSlot,
		removeFromBelt,
		putToSlot,
		putToBelt,
		putToRuck
	};
}

typedef enum_exporter<InventoryPlaceChange::EnumItemPlaceChange> CInventoryPlaceChange;
add_to_type_list(CInventoryPlaceChange)
#undef script_type_list
#define script_type_list save_type_list(CInventoryPlaceChange)

#endif