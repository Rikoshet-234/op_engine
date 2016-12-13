#ifndef inventory_slots_script_enum_h
#define inventory_slots_script_enum_h

#include "script_export_space.h"
#include "inventory_space.h"

namespace InventorySlots 
{
	enum InventorySlotsEnum //only!!!! for export to scripts...f#$*ng way.
	{
		KNIFE			=	KNIFE_SLOT,		
		PISTOL			=	PISTOL_SLOT,
		RIFLE			=	RIFLE_SLOT,
		GRENADE			=	GRENADE_SLOT,
		APPARATUS		=	APPARATUS_SLOT,
		BOLT			=	BOLT_SLOT,
		OUTFIT			=	OUTFIT_SLOT,
		PDA				=	PDA_SLOT,
		DETECTOR_ARTS	=	DETECTOR_ARTS_SLOT,
		DETECTOR_ANOMS	=	DETECTOR_ANOM_SLOT,
		TORCH			=	TORCH_SLOT,
		ARTEFACT		=	ARTEFACT_SLOT,
		PNV				=	PNV_SLOT,
		SHOTGUN			=	SHOTGUN_SLOT,
		BIODEV			=	BIODEV_SLOT,
		RUCK			=	SLOTS_TOTAL+1,
		BELT			=	RUCK+1,
		NO_ACT_SLOT		=	NO_ACTIVE_SLOT
	};
}

typedef enum_exporter<InventorySlots::InventorySlotsEnum> CInventorySlots;
add_to_type_list(CInventorySlots)
#undef script_type_list
#define script_type_list save_type_list(CInventorySlots)

#endif