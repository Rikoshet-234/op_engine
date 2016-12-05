////////////////////////////////////////////////////////////////////////////
//	Module 		: alife_keyval_registry.h
//	Created 	: 03.12.2016
//  Modified 	: 03.12.2016
//	Author		: jarni
//	Description : ALife key-value pairs registry
////////////////////////////////////////////////////////////////////////////

#pragma once

#include "script_export_space.h"

#pragma warning(push)
#pragma warning(disable:4995)
#include <malloc.h>
#pragma warning(pop)

class CALifeKeyvalRegistry 
{
	struct SKeyValItem
	{
		SKeyValItem(const shared_str& k, const shared_str& v) : key(k), value(v) {}
		shared_str key;
		shared_str value;
	};

public:
	typedef xr_hashmultimap<u32, SKeyValItem> TKeyVals;

protected:
	TKeyVals m_keyvals;

public:
	CALifeKeyvalRegistry();
	virtual ~CALifeKeyvalRegistry();
	
	void save(IWriter &memory_stream);
	void load(IReader &memory_stream);

	LPCSTR get(LPCSTR key);
	void set(LPCSTR key, LPCSTR value);
	
	DECLARE_SCRIPT_REGISTER_FUNCTION
};

add_to_type_list(CALifeKeyvalRegistry)
#undef script_type_list
#define script_type_list save_type_list(CALifeKeyvalRegistry)