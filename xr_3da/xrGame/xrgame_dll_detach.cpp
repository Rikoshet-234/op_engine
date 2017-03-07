#include "pch_script.h"
#include "ai_space.h"
#include "object_factory.h"
#include "ai/monsters/ai_monster_squad_manager.h"
#include "string_table.h"

#include "entity_alive.h"
#include "ui/UIInventoryUtilities.h"
#include "UI/UIXmlInit.h"

#include "InfoPortion.h"
#include "PhraseDialog.h"
#include "GameTask.h"
#include "encyclopedia_article.h"

#include "character_info.h"
#include "specific_character.h"
#include "character_community.h"
#include "monster_community.h"
#include "character_rank.h"
#include "character_reputation.h"

#include "profiler.h"

#include "sound_collection_storage.h"
#include "relation_registry.h"

typedef xr_vector<std::pair<shared_str,int> >	STORY_PAIRS;
extern STORY_PAIRS								story_ids;
extern STORY_PAIRS								spawn_story_ids;

extern void show_smart_cast_stats					();
extern void clear_smart_cast_stats					();
extern void release_smart_cast_stats				();
extern void dump_list_wnd							();
extern void dump_list_lines							();
extern void dump_list_sublines						();
extern void clean_wnd_rects							();
extern void dump_list_xmls							();
extern void CreateUIGeom							();
extern void DestroyUIGeom							();

#include "../IGame_Persistent.h"

#include "../xrCore/OPFuncs/global_timers.h"

void init_game_globals()
{
	CreateUIGeom									();
	//TSE_ENABLE("init");
	if(!g_dedicated_server)
	{
		TSP_BEGIN("CInfoPortion", "init");
		CInfoPortion::InitInternal						();
		TSP_END("CInfoPortion","init");
		TSP_BEGIN("CEncyclopediaArticle", "init");
		CEncyclopediaArticle::InitInternal				();
		TSP_END("CEncyclopediaArticle", "init");
		TSP_BEGIN("CPhraseDialog", "init");
		CPhraseDialog::InitInternal						();
		TSP_END("CPhraseDialog",	"init");
		TSP_BEGIN("InventoryUtilities::CreateShaders", "init");
		InventoryUtilities::CreateShaders				();
		TSP_END("InventoryUtilities::CreateShaders",	"init");
	};
	TSP_BEGIN("CCharacterInfo", "init");
	CCharacterInfo::InitInternal					();
	TSP_END("CCharacterInfo",	"init");
	TSP_BEGIN("CSpecificCharacter", "init");
	CSpecificCharacter::InitInternal				();
	TSP_END("CSpecificCharacter",	"init");
	TSP_BEGIN("CHARACTER_COMMUNITY", "init");
	CHARACTER_COMMUNITY::InitInternal				();
	TSP_END("CHARACTER_COMMUNITY",	"init");
	TSP_BEGIN("CHARACTER_RANK", "init");
	CHARACTER_RANK::InitInternal					();
	TSP_END("CHARACTER_RANK",	"init");
	TSP_BEGIN("CHARACTER_REPUTATION", "init");
	CHARACTER_REPUTATION::InitInternal				();
	TSP_END("CHARACTER_REPUTATION",	"init");
	TSP_BEGIN("MONSTER_COMMUNITY", "init");
	MONSTER_COMMUNITY::InitInternal					();
	TSP_END("MONSTER_COMMUNITY",	"init");
	TSE_DISABLE("init");
	TSP_PRINT();
	TSS_PRINT("g_iiFindBegin");
	TSS_PRINT("g_iiFindEnd");
}

extern CUIXml*	g_gameTaskXml;
extern CUIXml*	g_uiSpotXml;

extern void destroy_lua_wpn_params	();

void clean_game_globals()
{
	destroy_lua_wpn_params							();
	// destroy ai space
	xr_delete										(g_ai_space);
	// destroy object factory
	xr_delete										(g_object_factory);
	// destroy monster squad global var
	xr_delete										(g_monster_squad);

	story_ids.clear									();
	spawn_story_ids.clear							();

	if(!g_dedicated_server)
	{
		CInfoPortion::DeleteSharedData					();
		CInfoPortion::DeleteIdToIndexData				();

		CEncyclopediaArticle::DeleteSharedData			();
		CEncyclopediaArticle::DeleteIdToIndexData		();

		CPhraseDialog::DeleteSharedData					();
		CPhraseDialog::DeleteIdToIndexData				();
		
		InventoryUtilities::DestroyShaders				();
	}
	CCharacterInfo::DeleteSharedData				();
	CCharacterInfo::DeleteIdToIndexData				();
	
	CSpecificCharacter::DeleteSharedData			();
	CSpecificCharacter::DeleteIdToIndexData			();

	CHARACTER_COMMUNITY::DeleteIdToIndexData		();
	CHARACTER_RANK::DeleteIdToIndexData				();
	CHARACTER_REPUTATION::DeleteIdToIndexData		();
	MONSTER_COMMUNITY::DeleteIdToIndexData			();


	//static shader for blood
	CEntityAlive::UnloadBloodyWallmarks				();
	CEntityAlive::UnloadFireParticles				();
	//очищение памяти таблицы строк
	CStringTable::Destroy							();
	// Очищение таблицы цветов
	CUIXmlInit::DeleteColorDefs						();
	// Очищение таблицы идентификаторов рангов и отношений сталкеров
	InventoryUtilities::ClearCharacterInfoStrings	();

	xr_delete										(g_sound_collection_storage);
	
#ifdef DEBUG
	xr_delete										(g_profiler);
	release_smart_cast_stats						();
#endif

	RELATION_REGISTRY::clear_relation_registry		();

	dump_list_wnd									();
	dump_list_lines									();
	dump_list_sublines								();
	clean_wnd_rects									();
	xr_delete										(g_gameTaskXml);
	xr_delete										(g_uiSpotXml);
	dump_list_xmls									();
	DestroyUIGeom									();
}