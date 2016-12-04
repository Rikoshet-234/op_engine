////////////////////////////////////////////////////////////////////////////
//	Module 		: alife_keyval_registry.cpp
//	Created 	: 03.12.2016
//  Modified 	: 03.12.2016
//	Author		: jarni
//	Description : ALife key-value pairs registry
////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "alife_keyval_registry.h"
#include "alife_space.h"
#include "script_engine.h"
#include "ai_space.h"

CALifeKeyvalRegistry::CALifeKeyvalRegistry()
{
}

CALifeKeyvalRegistry::~CALifeKeyvalRegistry	()
{
}

LPCSTR CALifeKeyvalRegistry::get(LPCSTR key)
{
	if (key == nullptr)
	{
		ai().script_engine().print_stack();
		FATAL("[keyval] key must not be null");
	}

	shared_str skey(key);

	auto range = m_keyvals.equal_range(skey._get()->dwCRC);
	for(auto i = range.first; i != range.second; ++i)
	{
		if(i->second.key == key)
		{
			return i->second.value.c_str();
		}
	}
	
	return nullptr;
}

void CALifeKeyvalRegistry::set(LPCSTR key, LPCSTR value)
{
	if (key == nullptr)
	{
		ai().script_engine().print_stack();
		FATAL("[keyval] key must not be null");
	}

	shared_str skey(key);

	auto range = m_keyvals.equal_range(skey._get()->dwCRC);
	bool found = false;
	for(auto i = range.first; i != range.second && !found; ++i)
	{
		if(i->second.key == key)
		{
			if (value != nullptr)
			{
				i->second.value = value;
			}
			else
			{
				m_keyvals.erase(i);
			}
			found = true;
		}
	}
	
	if (!found && value != nullptr)
	{
		m_keyvals.insert(TKeyVals::value_type(skey._get()->dwCRC, SKeyValItem(skey, value)));
	}
}

void CALifeKeyvalRegistry::save(IWriter &memory_stream)
{
	Msg("* Saving key-value pairs...");

	memory_stream.open_chunk(KEYVAL_CHUNK_DATA);
	memory_stream.w_u32(u32(m_keyvals.size()));//! Reserve space for # of pairs

	for (auto i = m_keyvals.begin(); i != m_keyvals.end(); ++i)
	{
		memory_stream.w_stringZ(i->second.key.c_str());
		memory_stream.w_stringZ(i->second.value.c_str());
	}
	
	memory_stream.close_chunk();
	
	Msg("* %d key-value pairs were successfully saved", m_keyvals.size());
}

void CALifeKeyvalRegistry::load(IReader &memory_stream)
{ 
	CTimer t;
	t.Start();
	Msg("* Loading key-value pairs...");
	
	m_keyvals.clear();
	if (memory_stream.find_chunk(KEYVAL_CHUNK_DATA))
	{
		u32 count = memory_stream.r_u32();
		for (u32 i = 0; i < count; ++i) 
		{
			shared_str key, value;
			memory_stream.r_stringZ(key);
			memory_stream.r_stringZ(value);
			m_keyvals.insert(TKeyVals::value_type(key._get()->dwCRC, SKeyValItem(key, value)));
		}
	}

	Msg("* %u key-value pairs were successfully loaded (%2.3fs)", (u32)m_keyvals.size(), t.GetElapsed_sec());
}
