////////////////////////////////////////////////////////////////////////////
//	Module 		: alife_storage_manager.cpp
//	Created 	: 25.12.2002
//  Modified 	: 12.05.2004
//	Author		: Dmitriy Iassenev
//	Description : ALife Simulator storage manager
////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "alife_storage_manager.h"
#include "alife_simulator_header.h"
#include "alife_time_manager.h"
#include "alife_spawn_registry.h"
#include "alife_object_registry.h"
#include "alife_graph_registry.h"
#include "alife_group_registry.h"
#include "alife_registry_container.h"
#include "xrserver.h"
#include "level.h"
#include "../x_ray.h"
#include "saved_game_wrapper.h"
#include "string_table.h"
#include "../igame_persistent.h"

using namespace ALife;

extern string_path g_last_saved_game;

CALifeStorageManager::~CALifeStorageManager	()
{
}

void CALifeStorageManager::save	(LPCSTR save_name, bool update_name)
{
	strcpy_s					(g_last_saved_game,sizeof(g_last_saved_game),save_name);

	string_path					save;
	strcpy						(save,m_save_name);
	if (save_name) {
		strconcat				(sizeof(m_save_name),m_save_name,save_name,SAVE_EXTENSION);
	}
	else {
		if (!xr_strlen(m_save_name)) {
			Log					("There is no file name specified!");
			return;
		}
	}

	u32							source_count;
	u32							dest_count;
	void						*dest_data;
	{
		CMemoryWriter			stream;
		header().save			(stream);
		time_manager().save		(stream);
		spawns().save			(stream);
		objects().save			(stream);
		registry().save			(stream);

		source_count			= stream.tell();
		void					*source_data = stream.pointer();
		dest_count				= rtc_csize(source_count);
		dest_data				= xr_malloc(dest_count);
		dest_count				= rtc_compress(dest_data,dest_count,source_data,source_count);
	}

	string_path					temp;
	FS.update_path				(temp,"$game_saves$",m_save_name);
	IWriter						*writer = FS.w_open(temp);
	writer->w_u32				(u32(-1));
	writer->w_u32				(ALIFE_VERSION);
	
	writer->w_u32				(source_count);
	writer->w					(dest_data,dest_count);
	xr_free						(dest_data);
	FS.w_close					(writer);
#ifdef DEBUG
	Msg							("* Game %s is successfully saved to file '%s' (%d bytes compressed to %d)",m_save_name,temp,source_count,dest_count + 4);
#else // DEBUG
	Msg							("* Game %s is successfully saved to file '%s'",m_save_name,temp);
#endif // DEBUG

	if (!update_name)
		strcpy					(m_save_name,save);
}

bool g_measure = false;
CTimerStat g_dynamic_td;
CTimerStat g_dynamic;
CTimerStat g_dynamic_bu;
CTimerStat g_human;
CTimerStat g_human_inherited;
CTimerStat g_human_brain;
CTimerStat g_human_specific;
extern CTimerStat g_brain_select_task;
extern CTimerStat g_brain_st;
extern CTimerStat g_brain_al;
extern CTimerStat g_brain_for;
extern CTimerStat g_brain_for_en;
extern CTimerStat g_brain_reg;
extern CTimerStat g_has_info;
extern CTimerStat g_has_info_registry;
extern CTimerStat g_has_info_find_if;

void CALifeStorageManager::load	(void *buffer, const u32 &buffer_size, LPCSTR file_name)
{
	IReader						source(buffer,buffer_size);
	header().load				(source);
	time_manager().load			(source);
	spawns().load				(source,file_name);

#ifdef PRIQUEL
	graph().on_load				();
#endif // PRIQUEL

	objects().load				(source);

	VERIFY						(can_register_objects());
	can_register_objects		(false);
	CALifeObjectRegistry::OBJECT_REGISTRY::iterator	B = objects().objects().begin();
	CALifeObjectRegistry::OBJECT_REGISTRY::iterator	E = objects().objects().end();
	CALifeObjectRegistry::OBJECT_REGISTRY::iterator	I;
	for (I = B; I != E; ++I) {
		ALife::_OBJECT_ID		id = (*I).second->ID;
		(*I).second->ID			= server().PerformIDgen(id);
		VERIFY					(id == (*I).second->ID);
		register_object			((*I).second,false);
	} 
	registry().load				(source);

	can_register_objects		(true);
	
	//g_measure = true;
	CTimer t;
	t.Start();
	int i = 0;
	double new_max = 0.;
	for (I = B; I != E; ++I, ++i)
	{
		if (g_measure)
		{
			g_dynamic_td.Begin();
		}
		(*I).second->on_register();
		if(g_measure)
		{
			g_dynamic_bu.End();
			if (g_dynamic_bu.GetMax() > new_max)
			{
				new_max = g_dynamic_bu.GetMax();
				Msg("*  Dynam_BU: New max(%d) = %2.3f ms", i, new_max);
			}
		}
	}
	Msg("* %u objects on_register'ed (%2.3fs)", objects().objects().size(), t.GetElapsed_sec());
	Msg("*  Dynam_TD: Count = %u, Elapsed = %I64u ms, Average = %2.3f ms, Max = %2.3f ms, Min = %2.3f ms"
		, g_dynamic_td.GetCount()
		, g_dynamic_td.GetElapsed_ms()
		, g_dynamic_td.GetAvg()
		, g_dynamic_td.GetMax()
		, g_dynamic_td.GetMin());
	Msg("*  Dynamic: Count = %u, Elapsed = %I64u ms, Average = %2.3f ms, Max = %2.3f ms, Min = %2.3f ms"
		, g_dynamic.GetCount()
		, g_dynamic.GetElapsed_ms()
		, g_dynamic.GetAvg()
		, g_dynamic.GetMax()
		, g_dynamic.GetMin());
	Msg("*    Dynam_BR_ST: Count = %u, Elapsed = %I64u ms, Average = %2.3f ms, Max = %2.3f ms, Min = %2.3f ms"
		, g_brain_st.GetCount()
		, g_brain_st.GetElapsed_ms()
		, g_brain_st.GetAvg()
		, g_brain_st.GetMax()
		, g_brain_st.GetMin());
	Msg("*    Dynam_BR_AL: Count = %u, Elapsed = %I64u ms, Average = %2.3f ms, Max = %2.3f ms, Min = %2.3f ms"
		, g_brain_al.GetCount()
		, g_brain_al.GetElapsed_ms()
		, g_brain_al.GetAvg()
		, g_brain_al.GetMax()
		, g_brain_al.GetMin());
	Msg("*       HasInfo::registry: Count = %u, Elapsed = %I64u ms, Average = %2.3f ms, Max = %2.3f ms, Min = %2.3f ms"
		, g_has_info_registry.GetCount()
		, g_has_info_registry.GetElapsed_ms()
		, g_has_info_registry.GetAvg()
		, g_has_info_registry.GetMax()
		, g_has_info_registry.GetMin());
	Msg("*       HasInfo::find_if: Count = %u, Elapsed = %I64u ms, Average = %2.3f ms, Max = %2.3f ms, Min = %2.3f ms"
		, g_has_info_find_if.GetCount()
		, g_has_info_find_if.GetElapsed_ms()
		, g_has_info_find_if.GetAvg()
		, g_has_info_find_if.GetMax()
		, g_has_info_find_if.GetMin());
	Msg("*      HasInfo: Count = %u, Elapsed = %I64u ms, Average = %2.3f ms, Max = %2.3f ms, Min = %2.3f ms"
		, g_has_info.GetCount()
		, g_has_info.GetElapsed_ms()
		, g_has_info.GetAvg()
		, g_has_info.GetMax()
		, g_has_info.GetMin());
	Msg("*     Dynam_BR_FO_EN: Count = %u, Elapsed = %I64u ms, Average = %2.3f ms, Max = %2.3f ms, Min = %2.3f ms"
		, g_brain_for_en.GetCount()
		, g_brain_for_en.GetElapsed_ms()
		, g_brain_for_en.GetAvg()
		, g_brain_for_en.GetMax()
		, g_brain_for_en.GetMin());
	
	Msg("*    Dynam_BR_FO: Count = %u, Elapsed = %I64u ms, Average = %2.3f ms, Max = %2.3f ms, Min = %2.3f ms"
		, g_brain_for.GetCount()
		, g_brain_for.GetElapsed_ms()
		, g_brain_for.GetAvg()
		, g_brain_for.GetMax()
		, g_brain_for.GetMin());
	Msg("*    Dynam_BR_RE: Count = %u, Elapsed = %I64u ms, Average = %2.3f ms, Max = %2.3f ms, Min = %2.3f ms"
		, g_brain_reg.GetCount()
		, g_brain_reg.GetElapsed_ms()
		, g_brain_reg.GetAvg()
		, g_brain_reg.GetMax()
		, g_brain_reg.GetMin());
	Msg("*   Dynam_BR: Count = %u, Elapsed = %I64u ms, Average = %2.3f ms, Max = %2.3f ms, Min = %2.3f ms"
		, g_brain_select_task.GetCount()
		, g_brain_select_task.GetElapsed_ms()
		, g_brain_select_task.GetAvg()
		, g_brain_select_task.GetMax()
		, g_brain_select_task.GetMin());
	Msg("*  Dynam_BU: Count = %u, Elapsed = %I64u ms, Average = %2.3f ms, Max = %2.3f ms, Min = %2.3f ms"
		, g_dynamic_bu.GetCount()
		, g_dynamic_bu.GetElapsed_ms()
		, g_dynamic_bu.GetAvg()
		, g_dynamic_bu.GetMax()
		, g_dynamic_bu.GetMin());
	Msg("*   HumanI: Count = %u, Elapsed = %I64u ms, Average = %2.3f ms, Max = %2.3f ms, Min = %2.3f ms"
		, g_human_inherited.GetCount()
		, g_human_inherited.GetElapsed_ms()
		, g_human_inherited.GetAvg()
		, g_human_inherited.GetMax()
		, g_human_inherited.GetMin());
	Msg("*   HumanB: Count = %u, Elapsed = %I64u ms, Average = %2.3f ms, Max = %2.3f ms, Min = %2.3f ms"
		, g_human_brain.GetCount()
		, g_human_brain.GetElapsed_ms()
		, g_human_brain.GetAvg()
		, g_human_brain.GetMax()
		, g_human_brain.GetMin());
	Msg("*   HumanS: Count = %u, Elapsed = %I64u ms, Average = %2.3f ms, Max = %2.3f ms, Min = %2.3f ms"
		, g_human_specific.GetCount()
		, g_human_specific.GetElapsed_ms()
		, g_human_specific.GetAvg()
		, g_human_specific.GetMax()
		, g_human_specific.GetMin());
	Msg("*  Human__: Count = %u, Elapsed = %I64u ms, Average = %2.3f ms, Max = %2.3f ms, Min = %2.3f ms"
		, g_human.GetCount()
		, g_human.GetElapsed_ms()
		, g_human.GetAvg()
		, g_human.GetMax()
		, g_human.GetMin());
	g_measure = false;
}

bool CALifeStorageManager::load	(LPCSTR save_name)
{
	CTimer						timer;
	timer.Start					();
	string256					save;
	strcpy						(save,m_save_name);
	if (!save_name) {
		if (!xr_strlen(m_save_name))
			R_ASSERT2			(false,"There is no file name specified!");
	}
	else
		strconcat				(sizeof(m_save_name),m_save_name,save_name,SAVE_EXTENSION);
	string_path					file_name;
	FS.update_path				(file_name,"$game_saves$",m_save_name);

	IReader						*stream;
	stream						= FS.r_open(file_name);
	if (!stream) {
		Msg						("* Cannot find saved game %s",file_name);
		strcpy					(m_save_name,save);
		return					(false);
	}

	CHECK_OR_EXIT				(CSavedGameWrapper::valid_saved_game(*stream),make_string("%s\nSaved game version mismatch or saved game is corrupted",file_name));

	string512					temp;
	strconcat					(sizeof(temp),temp,CStringTable().translate("st_loading_saved_game").c_str()," \"",save_name,SAVE_EXTENSION,"\"");
	g_pGamePersistent->LoadTitle(temp);

	unload						();
	reload						(m_section);

	u32							source_count = stream->r_u32();
	void						*source_data = xr_malloc(source_count);
	rtc_decompress				(source_data,source_count,stream->pointer(),stream->length() - 3*sizeof(u32));
	FS.r_close					(stream);
	load						(source_data, source_count, file_name);
	xr_free						(source_data);

	groups().on_after_game_load	();

	VERIFY						(graph().actor());
	
	Msg							("* Game %s is successfully loaded from file '%s' (%.3fs)",save_name, file_name,timer.GetElapsed_sec());

	return						(true);
}

void CALifeStorageManager::save	(NET_Packet &net_packet)
{
	prepare_objects_for_save	();

	shared_str					game_name;
	net_packet.r_stringZ		(game_name);
	save						(*game_name,!!net_packet.r_u8());
}

void CALifeStorageManager::prepare_objects_for_save	()
{
	Level().ClientSend			();
	Level().ClientSave			();
}
