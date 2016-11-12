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

#ifndef TS_ENABLE
//#define TS_ENABLE
#endif

#ifdef TS_ENABLE
#define TS_DECLARE(x) CTimerStat x
#define TS_BEGIN(x) x.Begin()
#define TS_END(x) x.End()
#define TS_RESET(x) x.Reset()
#define TS_P(x,name) Msg( name ": Count = %u, Elapsed = %I64u ms, Average = %2.3f ms, Max = %2.3f ms, Min = %2.3f ms", x.GetCount(), x.GetElapsed_ms(), x.GetAvg(), x.GetMax(), x.GetMin())
#define TS_PR(x,name) Msg( name ": Count = %u, Elapsed = %I64u ms, Average = %2.3f ms, Max = %2.3f ms, Min = %2.3f ms", x.GetCount(), x.GetElapsed_ms(), x.GetAvg(), x.GetMax(), x.GetMin()); x.Reset()
#define TS_EPR(x,name) x.End(); Msg( name ": Count = %u, Elapsed = %I64u ms, Average = %2.3f ms, Max = %2.3f ms, Min = %2.3f ms", x.GetCount(), x.GetElapsed_ms(), x.GetAvg(), x.GetMax(), x.GetMin()); x.Reset()
#else
#define TS_DECLARE(x)
#define TS_BEGIN(x)
#define TS_END(x)
#define TS_RESET(x)
#define TS_P(x,name)
#define TS_EPR(x,name)
#define TS_PR(x,name)
#endif

TS_DECLARE(g_initInternal);
TS_DECLARE(g_iiForOuter);
TS_DECLARE(g_iiForInner);
TS_DECLARE(g_iiFIFind);

void init_game_globals()
{
	CreateUIGeom									();

	if(!g_dedicated_server)
	{
		TS_BEGIN(g_initInternal);
		CInfoPortion::InitInternal						();
		TS_EPR(g_initInternal,	"CInfoPortion");
		TS_PR(g_iiForOuter,		"CInfoPortion::FO");
		TS_PR(g_iiForInner,		"CInfoPortion::FI");
		TS_PR(g_iiFIFind,		"CInfoPortion::FF");
		TS_BEGIN(g_initInternal);
		CEncyclopediaArticle::InitInternal				();
		TS_EPR(g_initInternal,	"CEncyclopediaArticle");
		TS_PR(g_iiForOuter,		"CEncyclopediaArticle::FO");
		TS_PR(g_iiForInner,		"CEncyclopediaArticle::FI");
		TS_PR(g_iiFIFind,		"CEncyclopediaArticle::FF");
		TS_BEGIN(g_initInternal);
		CPhraseDialog::InitInternal						();
		TS_EPR(g_initInternal,	"CPhraseDialog");
		TS_PR(g_iiForOuter,		"CPhraseDialog::FO");
		TS_PR(g_iiForInner,		"CPhraseDialog::FI");
		TS_PR(g_iiFIFind,		"CPhraseDialog::FF");
		TS_BEGIN(g_initInternal);
		InventoryUtilities::CreateShaders				();
		TS_EPR(g_initInternal,	"InventoryUtilities::CreateShaders");
	};
	TS_BEGIN(g_initInternal);
	CCharacterInfo::InitInternal					();
	TS_EPR(g_initInternal,	"CCharacterInfo");
	TS_PR(g_iiForOuter,		"CCharacterInfo::FO");
	TS_PR(g_iiForInner,		"CCharacterInfo::FI");
	TS_PR(g_iiFIFind,		"CCharacterInfo::FF");
	TS_BEGIN(g_initInternal);
	CSpecificCharacter::InitInternal				();
	TS_EPR(g_initInternal,	"CSpecificCharacter");
	TS_PR(g_iiForOuter,		"CSpecificCharacter::FO");
	TS_PR(g_iiForInner,		"CSpecificCharacter::FI");
	TS_PR(g_iiFIFind,		"CSpecificCharacter::FF");
	TS_BEGIN(g_initInternal);
	CHARACTER_COMMUNITY::InitInternal				();
	TS_EPR(g_initInternal,	"CHARACTER_COMMUNITY");
	TS_PR(g_iiForOuter,		"CHARACTER_COMMUNITY::FO");
	TS_PR(g_iiForInner,		"CHARACTER_COMMUNITY::FI");
	TS_PR(g_iiFIFind,		"CHARACTER_COMMUNITY::FF");
	TS_BEGIN(g_initInternal);
	CHARACTER_RANK::InitInternal					();
	TS_EPR(g_initInternal,	"CHARACTER_RANK");
	TS_PR(g_iiForOuter,		"CHARACTER_RANK::FO");
	TS_PR(g_iiForInner,		"CHARACTER_RANK::FI");
	TS_PR(g_iiFIFind,		"CHARACTER_RANK::FF");
	TS_BEGIN(g_initInternal);
	CHARACTER_REPUTATION::InitInternal				();
	TS_EPR(g_initInternal,	"CHARACTER_REPUTATION");
	TS_PR(g_iiForOuter,		"CHARACTER_REPUTATION::FO");
	TS_PR(g_iiForInner,		"CHARACTER_REPUTATION::FI");
	TS_PR(g_iiFIFind,		"CHARACTER_REPUTATION::FF");
	TS_BEGIN(g_initInternal);
	MONSTER_COMMUNITY::InitInternal					();
	TS_EPR(g_initInternal,	"MONSTER_COMMUNITY");
	TS_PR(g_iiForOuter,		"MONSTER_COMMUNITY::FO");
	TS_PR(g_iiForInner,		"MONSTER_COMMUNITY::FI");
	TS_PR(g_iiFIFind,		"MONSTER_COMMUNITY::FF");
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