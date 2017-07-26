#include "stdafx.h"
#include "QuickSlots.h"
#include "xr_level_controller.h"
#include "alife_space.h"
#include "inventory_item.h"

CQuickSlots* QuickSlotManager = nullptr;
#define SFV_V1 1
#define SFV_V2 2
#define CASF_VERSION SFV_V2

CQuickSlots::slot::slot(const shared_str& item_section, bool empty): itemSection(item_section), emptySlot(empty)
{
}

CQuickSlots::slot::slot(): itemSection(""), emptySlot(true)
{
}

void CQuickSlots::slot::set(CInventoryItem* item)
{
	if (item)
	{
		emptySlot = false;
		itemSection = item->object().cNameSect();
	}
	else
	{
		emptySlot = true;
		itemSection = nullptr;
	}
}

void CQuickSlots::slot::use()
{
	Msg("slot [%s] not empty!!!", itemSection);
}

void CQuickSlots::slot::save(IWriter& memory_stream, u8 version)
{
	memory_stream.w_u8(emptySlot?0:1);
	if (!isEmpty())
	{
		memory_stream.w_stringZ(itemSection);
	}
}

void CQuickSlots::slot::load(IReader& memory_stream, u8 version)
{
	emptySlot = memory_stream.r_u8() == 0 ? true : false;
	if (!isEmpty())
	{
		memory_stream.r_stringZ(itemSection);
	}
}

CQuickSlots::CQuickSlots()
{
	quickSlots.insert(mk_pair(kUSE_QUICK_SLOT0, slot()));
	quickSlots.insert(mk_pair(kUSE_QUICK_SLOT1, slot()));
	quickSlots.insert(mk_pair(kUSE_QUICK_SLOT2, slot()));
	quickSlots.insert(mk_pair(kUSE_QUICK_SLOT3, slot()));
}

void CQuickSlots::UseSlot(int slotCmd)
{
	Msg("quick slot use [%d]", slotCmd);
	auto slotIt = quickSlots.find(slotCmd);
	if (slotIt == quickSlots.end() || (slotIt != quickSlots.end() && slotIt->second.isEmpty()))
		return;
	slotIt->second.use();
}

void CQuickSlots::SetSlot(int slotId, CInventoryItem* item)
{
	auto slotIt = quickSlots.find(slotId);
	if (slotIt == quickSlots.end())
		return;
	slotIt->second.set(item);
}

void CQuickSlots::saveSlots(IWriter &memory_stream)
{
	Msg("* Saving quick slots data...");
	memory_stream.open_chunk(QUICKSLOTS_CHUNK_DATA);
	memory_stream.w_u8(CASF_VERSION);
	std::for_each(quickSlots.begin(), quickSlots.end(), [&](std::pair<int,slot> pair) {pair.second.save(memory_stream, CASF_VERSION); });
	memory_stream.close_chunk();
}

void CQuickSlots::loadSlots(IReader &memory_stream)
{
	Msg("* Loading quick slots data...");
	if (memory_stream.find_chunk(QUICKSLOTS_CHUNK_DATA))
	{
		u8 version = memory_stream.r_u8();
		switch (version)
		{
		case SFV_V1:
		case SFV_V2:
			std::for_each(quickSlots.begin(), quickSlots.end(), [&](std::pair<int, slot> pair) {pair.second.load(memory_stream,version); });
			break;
		default:
			Msg("! ERROR QSM: Unsupported version from save file!");
			break;
		}
	}
}
