////////////////////////////////////////////////////////////////////////////
//	Module 		: alife_keyval_registry.h
//	Created 	: 03.12.2016
//  Modified 	: 03.12.2016
//	Author		: jarni
//	Description : ALife key-value pairs registry
////////////////////////////////////////////////////////////////////////////

#pragma once

#include "script_export_space.h"
#include <luabind/luabind.hpp>

#pragma warning(push)
#pragma warning(disable:4995)
#include <malloc.h>
#pragma warning(pop)

class CALifeKeyvalContainer
{
	struct SKeyValItem
	{
		SKeyValItem(const shared_str& k, const luabind::object& v) : key(k), value(v) {}
		shared_str key;
		luabind::object value;
	};

public:
	typedef xr_hashmultimap<u32, SKeyValItem> TKeyVals;

protected:
	TKeyVals m_keyvals;

public:
	CALifeKeyvalContainer();
	virtual ~CALifeKeyvalContainer();
	
	u32 save(IWriter &memory_stream);
	u32 load(IReader &memory_stream);

	luabind::object get(LPCSTR key);
	void set(LPCSTR key, luabind::object const& object);
	bool exist(LPCSTR key);
	void remove(LPCSTR key);
	void clear();
	luabind::object list();

	DECLARE_SCRIPT_REGISTER_FUNCTION
};

class CALifeKeyvalRegistry 
{
	struct SKeyContItem
	{
		SKeyContItem(const shared_str& k, CALifeKeyvalContainer* v) : key(k), value(v) {}
		shared_str key;
		CALifeKeyvalContainer* value;
	};
public:
	typedef xr_hashmultimap<u32, SKeyContItem> TKeyValContainers;

protected:
	CALifeKeyvalContainer m_generic;
	TKeyValContainers m_specific;

public:
	CALifeKeyvalRegistry();
	virtual ~CALifeKeyvalRegistry();
	
	void save(IWriter &memory_stream);
	void load(IReader &memory_stream);

	CALifeKeyvalContainer* container(LPCSTR name);
	void remove(LPCSTR key);
	luabind::object list();

protected:
	void removeall();
};

add_to_type_list(CALifeKeyvalContainer)
#undef script_type_list
#define script_type_list save_type_list(CALifeKeyvalContainer)