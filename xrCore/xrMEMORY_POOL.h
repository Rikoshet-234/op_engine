#ifndef xrMemory_POOLh
#define xrMemory_POOLh
#pragma once

class xrMemory;

class	MEMPOOL
{
#ifdef DEBUG_MEMORY_MANAGER
	friend class xrMemory;
#endif // DEBUG_MEMORY_MANAGER
private:
	xrCriticalSection	cs;
	u32					s_sector;		// large-memory sector size
	u32					s_element;		// element size, for example 32
	u32					s_count;		// element count = [s_sector/s_element]
	u32					s_offset;		// header size
	u32					block_count;	// block count
	u8*					list;
private:
	ICF void**			access			(void* P)	{ return (void**) ((void*)(P));	}
	void				block_create	();
	void				store_stat		(void* e);
	void				remove_stat		(void* e);
public:
	void				_initialize		(u32 _element, u32 _sector, u32 _header);
	void				_dump			(const void* corrupted_memory_item_ptr);
#ifdef PROFILE_CRITICAL_SECTIONS
	ICF					MEMPOOL			(): cs(MUTEX_PROFILE_ID(memory_pool)){}
#endif // PROFILE_CRITICAL_SECTIONS
	
	ICF u32				get_block_count	()	{ return block_count; }
	ICF u32				get_element		()	{ return s_element; }

	ICF void*			create			()
	{
		cs.Enter		();
		if (0==list)	block_create();

		void* E			= list;
		list			= (u8*)*access(list);
		store_stat(E);
		cs.Leave		();
		return			E;
	}
	ICF void			destroy			(void* &P)
	{
		cs.Enter		();
		*access(P)		= list;
		list			= (u8*)P;
		remove_stat(P);
		cs.Leave		();
	}
};
#endif
