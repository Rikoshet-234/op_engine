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
#include "script_thread.h"
#include "ai_space.h"
#include "xr_time.h"
#include "ui/UIInventoryUtilities.h"

#define KEYVALS_V0 ((u8)0)
#define KEYVALS_V1 ((u8)1) //small hack for skipped versions before begin versioning :)
#define KEYVALS_V2 ((u8)2) 
#define KEYVALS_V3 ((u8)3) //actual version, add store xrTime to keyvals
#define CURRENT_KEYVALS_VERSION KEYVALS_V3
#define LUA_TXRTIME 9 //LUA_TTHREAD has 8 

static void serialize(IWriter &memory_stream, const luabind::object& object)
{
	switch (object.type())
	{
	case LUA_TNIL:
		memory_stream.w_u8(LUA_TNIL);
		break;
	case LUA_TBOOLEAN:
		memory_stream.w_u8(LUA_TBOOLEAN);
		memory_stream.w_u8((u8)(luabind::object_cast<bool>(object) ? 1 : 0));
		break;
	case LUA_TNUMBER:
		memory_stream.w_u8(LUA_TNUMBER);
		memory_stream.w_float(luabind::object_cast<float>(object));
		break;
	case LUA_TSTRING:
		memory_stream.w_u8(LUA_TSTRING);
		memory_stream.w_stringZ(luabind::object_cast<LPCSTR>(object));
		break;
	case LUA_TTABLE:
		{
			memory_stream.w_u8(LUA_TTABLE);
			u32 size = 0;
			for(luabind::object::iterator iter=object.begin(); iter != object.end(); ++iter, ++size);
			memory_stream.w_u32(size);
			for(luabind::object::iterator iter=object.begin(); iter != object.end(); ++iter)
			{
				serialize(memory_stream, iter.key()); 
				serialize(memory_stream, *iter);
			}
		}
		break;
	case LUA_TUSERDATA:
		{
			xrTime *time = luabind::object_cast<xrTime*>(object);
			if (time)
			{
				memory_stream.w_u8(LUA_TXRTIME);
				memory_stream.w_u64(time->get_m_time());
				break;
			}
		}
	case LUA_TLIGHTUSERDATA:
	case LUA_TFUNCTION:
	case LUA_TTHREAD:
	default:
		{
			ai().script_engine().print_stack();
			FATAL("[keyval] trying to serialize unsupported lua type");
		}
		break;
	}
}

static void deserialize(IReader &memory_stream, luabind::object& container)
{
	const u8 type = memory_stream.r_u8();
	switch (type)
	{
		case LUA_TXRTIME:
			{
				u64 rtime = memory_stream.r_u64();
				container = xrTime(rtime);
			}
			break;
		case LUA_TNIL:
			container = luabind::object();
		break;
		case LUA_TBOOLEAN:
		{
			bool value = memory_stream.r_u8() != 0;
			container = luabind::object(container.lua_state(), value);
		}
		break;
		case LUA_TNUMBER:
		{
			float value = memory_stream.r_float();
			container = value;
		}
		break;
		case LUA_TSTRING:
		{
			shared_str value;
			memory_stream.r_stringZ(value);
			container = value.c_str();
		}
		break;
		case LUA_TTABLE:
		{
			u32 size = memory_stream.r_u32();
			container = luabind::newtable(container.lua_state());
			for (u32 i = 0; i < size; ++i)
			{
				luabind::object key(container.lua_state());
				deserialize(memory_stream, key);

				luabind::object value(container.lua_state());
				deserialize(memory_stream, value);
				
				container[key] = value;
			}
		}
		break;
		case LUA_TLIGHTUSERDATA:
		case LUA_TFUNCTION:
		case LUA_TUSERDATA:
		case LUA_TTHREAD:
		default:
		{
			ai().script_engine().print_stack();
			FATAL("[keyval] trying to serialize unsupported lua type");
		}
		break;
	}
}

lua_State* getCurrentLuaState()
{
	CScriptThread *luaThread=ai().script_engine().current_thread();
	lua_State* lua;
	if (luaThread)
		lua=luaThread->lua();
	else
		lua=ai().script_engine().lua();
	return lua;
}


CALifeKeyvalContainer::CALifeKeyvalContainer()
{
}

CALifeKeyvalContainer::~CALifeKeyvalContainer	()
{
}

luabind::object CALifeKeyvalContainer::get(LPCSTR key)
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
			return i->second.value;
		}
	}
	
	return luabind::object();
}

luabind::object CALifeKeyvalContainer::get(LPCSTR key, luabind::object const& def_value)
{
	luabind::object result=get(key);
	if ((!result.is_valid() || result.type()==LUA_TNIL) && def_value.type()!=LUA_TNIL)
	{
		set(key,def_value);
		return get(key);
	}
	if (!result.is_valid())
		return luabind::object();
	return result;
}

void CALifeKeyvalContainer::set(LPCSTR key, luabind::object const& value)
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
			i->second.value = value;
			found = true;
		}
	}
	
	if (!found && value.type() != LUA_TNIL)
	{
		m_keyvals.insert(TKeyVals::value_type(skey._get()->dwCRC, SKeyValItem(skey, value)));
	}
}

bool CALifeKeyvalContainer::exist(LPCSTR key)
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
			return true;
		}
	}
	return false;
}

void CALifeKeyvalContainer::remove(LPCSTR key)
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
			m_keyvals.erase(i);
			found = true;
		}
	}
}

void CALifeKeyvalContainer::clear()
{
	m_keyvals.clear();
}

luabind::object CALifeKeyvalContainer::list()
{
	luabind::object array = luabind::newtable(getCurrentLuaState());

	int ai = 1;
	for(auto i = m_keyvals.begin(); i != m_keyvals.end(); ++i)
	{
		array[ai++] = i->second.key.c_str();
	}
	
	return array;
}

u32 CALifeKeyvalContainer::save(IWriter &memory_stream)
{
	memory_stream.w_u32(u32(m_keyvals.size()));//! Reserve space for # of pairs

	for (auto i = m_keyvals.begin(); i != m_keyvals.end(); ++i)
	{
		memory_stream.w_stringZ(i->second.key.c_str());
		serialize(memory_stream, i->second.value);
	}

	return u32(m_keyvals.size());
}

u32 CALifeKeyvalContainer::load(IReader &memory_stream)
{ 
	m_keyvals.clear();
	u32 count = memory_stream.r_u32();
	lua_State* L = getCurrentLuaState();
	for (u32 i = 0; i < count; ++i) 
	{
		shared_str key;
		memory_stream.r_stringZ(key);
		luabind::object value(L);
		deserialize(memory_stream, value);
		m_keyvals.insert(TKeyVals::value_type(key._get()->dwCRC, SKeyValItem(key, value)));
	}
	return count;
}

CALifeKeyvalRegistry::CALifeKeyvalRegistry()
{
}

CALifeKeyvalRegistry::~CALifeKeyvalRegistry	()
{
	removeall();
}

void CALifeKeyvalRegistry::save(IWriter &memory_stream)
{
	Msg("* Saving key-value pairs...");

	u32 count = 0;

	memory_stream.open_chunk(KEYVAL_CHUNK_DATA);
	memory_stream.w_u8(CURRENT_KEYVALS_VERSION);
	memory_stream.w_u32(u32(m_specific.size() + 1));//! Reserve space for # of containers
	count += m_generic.save(memory_stream);
	for (auto i = m_specific.begin(); i != m_specific.end(); ++i)
	{
		memory_stream.w_stringZ(i->second.key.c_str());
		count += i->second.value->save(memory_stream);
	}
	
	memory_stream.close_chunk();
	
	Msg("* %u key-value pairs were successfully saved", count);
}

void CALifeKeyvalRegistry::load(IReader &memory_stream)
{ 
	CTimer t;
	t.Start();
	Msg("* Loading key-value pairs...");
	
	removeall();
	u32 countPairs = 0;
	if (memory_stream.find_chunk(KEYVAL_CHUNK_DATA))
	{
		u8 version = memory_stream.r_u8();
		R_ASSERT2(version <= CURRENT_KEYVALS_VERSION, "[keyvals] Can't load save with newer version of keyvals registry");
		
		if (version == KEYVALS_V2 || version == KEYVALS_V3)
		{
			u32 count = memory_stream.r_u32();
			if (count > 0)
			{
				countPairs += m_generic.load(memory_stream);
				for (--count; count > 0; --count) 
				{
					shared_str key;
					memory_stream.r_stringZ(key);
			
					CALifeKeyvalContainer* container = xr_new<CALifeKeyvalContainer>();
					countPairs += container->load(memory_stream);
					m_specific.insert(TKeyValContainers::value_type(key._get()->dwCRC, SKeyContItem(key, container)));
				}
			}
			else
			{
				//! Loaders for older versions
			}
		}
		else 
		{//! Chunk is empty
		}
	}

	Msg("* %u key-value pairs were successfully loaded (%2.3fs)", countPairs, t.GetElapsed_sec());
}

CALifeKeyvalContainer* CALifeKeyvalRegistry::container(LPCSTR name)
{ 
	CALifeKeyvalContainer* container = nullptr;
	if (name == nullptr)
	{
		container = &m_generic;
	}
	else
	{
		shared_str key(name);
		auto range = m_specific.equal_range(key._get()->dwCRC);
		for(auto i = range.first; i != range.second; ++i)
		{
			if(i->second.key == key)
			{
				return i->second.value;
			}
		}

		container = xr_new<CALifeKeyvalContainer>();
		R_ASSERT2(container != nullptr, "[keyvals] Out of memory");
		m_specific.insert(TKeyValContainers::value_type(key._get()->dwCRC, SKeyContItem(key, container)));
	}
	return container;
}

void CALifeKeyvalRegistry::remove(LPCSTR key)
{
	if (key == nullptr)
	{
		ai().script_engine().print_stack();
		FATAL("[keyval] name must not be null");
	}

	shared_str skey(key);
	auto range = m_specific.equal_range(skey._get()->dwCRC);
	for(auto i = range.first; i != range.second; ++i)
	{
		if(i->second.key == key)
		{
			xr_delete<CALifeKeyvalContainer>(i->second.value);
			m_specific.erase(i);
			break;
		}
	}
}

luabind::object CALifeKeyvalRegistry::list()
{
	luabind::object array = luabind::newtable(getCurrentLuaState());

	int ai = 1;
	for(auto i = m_specific.begin(); i != m_specific.end(); ++i)
	{
		array[ai++] = i->second.key.c_str();
	}
	
	return array;
}

void CALifeKeyvalRegistry::removeall()
{
	for (auto i = m_specific.begin(); i != m_specific.end(); ++i)
	{
		xr_delete<CALifeKeyvalContainer>(i->second.value);
	}
	m_specific.clear();
}