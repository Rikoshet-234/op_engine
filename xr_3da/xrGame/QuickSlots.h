#ifndef QuickSlots_h
#define QuickSlots_h
#pragma once


#define INVALID_SLOT_ID static_cast<u8>(-1)
class CQuickSlots
{
private:
	class slot
	{
	public:
		slot(const shared_str& item_section, bool empty);
		slot();

		void set(CInventoryItem* item);
		void use();
		void save(IWriter &memory_stream, u8 version);
		void load(IReader &memory_stream, u8 version);
		bool isEmpty() const { return emptySlot; }
	private:
		shared_str itemSection;
		bool emptySlot;

	};

	xr_map<int, slot> quickSlots;

public:
	CQuickSlots();
	void UseSlot(int slotCmd);
	void SetSlot(int slotId, CInventoryItem* item);
	void saveSlots(IWriter &memory_stream);
	void loadSlots(IReader &memory_stream);
};

extern CQuickSlots* QuickSlotManager;

#endif
