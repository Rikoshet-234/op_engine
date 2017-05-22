////////////////////////////////////////////////////////////////////////////
//	Module 		: script_engine_export.h
//	Created 	: 01.04.2004
//  Modified 	: 22.06.2004
//	Author		: Dmitriy Iassenev
//	Description : XRay Script Engine export
////////////////////////////////////////////////////////////////////////////

#pragma once

#include "script_fvector.h"
#include "script_fcolor.h"
#include "script_fmatrix.h"
#include "script_flags.h"
#include "script_engine.h"
#include "script_token_list.h"
#include "script_rtoken_list.h"
#include "script_ini_file.h"
#include "script_reader.h"
#include "script_writer.h"
#include "script_net_packet.h"
#include "object_factory.h"
#include "script_sound_type.h"
#include "xrServer_Objects_ALife_All.h"

#ifndef XRGAME_EXPORTS
#	include "script_properties_list_helper.h"
#else
#	include "base_client_classes.h"
#	include "alife_simulator.h"
#	include "script_hit.h"
#	include "script_monster_hit_info.h"
#	include "script_sound_info.h"
#	include "script_sound.h"
#	include "script_particles.h"
#	include "script_binder_object.h"
#	include "action_planner_action.h"
#	include "action_planner.h"
#	include "action_base.h"
#	include "property_storage.h"
#	include "property_evaluator.h"
#	include "script_world_state.h"
#	include "script_world_property.h"
#	include "ai/stalker/ai_stalker.h"
#	include "script_effector.h"
#	include "particlesobject.h"
#	include "level.h"
#	include "memory_space.h"
#	include "script_render_device.h"
#	include "script_game_object.h"
#	include "cover_point.h"
#	include "patrol_path_params.h"
#	include "particle_params.h"
#	include "script_movement_action.h"
#	include "script_watch_action.h"
#	include "script_animation_action.h"
#	include "script_sound_action.h"
#	include "script_particle_action.h"
#	include "script_object_action.h"
#	include "script_action_condition.h"
#	include "script_monster_action.h"
#	include "script_entity_action.h"
#	include "helicopter.h"
#	include "game_base.h"
#	include "game_sv_base.h"
#	include "game_sv_mp.h"
#	include "game_sv_deathmatch.h"
#	include "game_sv_mp_script.h"
#	include "game_cl_mp.h"
#	include "game_cl_mp_script.h"
#	include "UIGameCustom.h"
#	include "UIGame_custom_script.h"
#	include "UI/UIScriptWnd.h"
#	include "UI/UIStatic.h"
#	include "UI/UIButton.h"
#	include "UI/UIWindow.h"
#	include "UI/UIProgressBar.h"
#	include "UI/UIEditBox.h"
#	include "UI/UIMessageBox.h"
#	include "UI/UIPropertiesBox.h"
#	include "UI/UITabControl.h"
#	include "UI/UIListWnd.h"
#   include "UI/UIComboBox.h"
#	include "ui/UIOptionsManagerScript.h"
#	include "ui/UIMapInfo.h"
#	include "ScriptXmlInit.h"
#	include "script_ui_registrator.h"
#	include "key_binding_registrator.h"
#	include "fs_registrator.h"
#	include "console_registrator.h"
#	include "multihuds_registrator.h"
#	include	"PhysicsShell.h"
#	include	"HangingLamp.h"
#	include "holder_custom.h"
#	include "GameTask.h"
#	include "client_spawn_manager.h"
#	include "script_zone.h"
#	include "ef_storage.h"
#	include "ai/stalker/ai_stalker.h"
#	include "ai/monsters/bloodsucker/bloodsucker.h"
#	include "ai/monsters/boar/boar.h"
#	include "ai/monsters/dog/dog.h"
#	include "ai/monsters/flesh/flesh.h"
#	include "ai/monsters/pseudodog/pseudodog.h"
#	include "ai/monsters/pseudodog/psy_dog.h"
#	include "ai/monsters/burer/burer.h"
#	include "ai/monsters/chimera/chimera.h"
#	include "ai/monsters/pseudogigant/pseudo_gigant.h"
#	include "ai/monsters/poltergeist/poltergeist.h"
#	include "ai/monsters/zombie/zombie.h"
#	include "ai/monsters/fracture/fracture.h"
#	include "ai/monsters/snork/snork.h"
#	include "ai/monsters/controller/controller.h"
#	include "ai/monsters/cat/cat.h"
#	include "ai/monsters/tushkano/tushkano.h"
#	include "PhraseDialog_script.h"
#	ifdef	DEBUG
#		include "PHDebug.h"
#	endif
#	include "hit.h"
#	include "PHDestroyable.h"
#	include "car.h"
#	include "script_lanim.h"
#	include "artifact.h"
#	include "game_cl_single.h"
#	include "alife_human_brain.h"
#	include "alife_monster_brain.h"
#	include "alife_monster_movement_manager.h"
#	include "alife_monster_patrol_path_manager.h"
#	include "alife_monster_detail_path_manager.h"
#	include "alife_smart_terrain_task.h"
#	include "alife_keyval_registry.h"
#	include "game_graph.h"
#	include "smart_zone.h"
#	include "F1.h"
#	include "RGD5.h"
#	include "Scope.h"
#	include "StalkerOutfit.h"
#	include "Torch.h"
#	include "WeaponAK74.h"
#	include "WeaponBinoculars.h"
#	include "WeaponBM16.h"
#	include "WeaponFN2000.h"
#	include "WeaponFORT.h"
#	include "WeaponGroza.h"
#	include "WeaponHPSA.h"
#	include "WeaponKnife.h"
#	include "WeaponLR300.h"
#	include "WeaponPM.h"
#	include "WeaponRG6.h"
#	include "WeaponRPG7.h"
#	include "WeaponShotgun.h"
#	include "WeaponSVD.h"
#	include "WeaponSVU.h"
#	include "WeaponUSP45.h"
#	include "WeaponVal.h"
#	include "WeaponVintorez.h"
#	include "WeaponWalther.h"
#	include "saved_game_wrapper.h"
#	include "mincer.h"
#	include "mosquitobald.h"
#	include "HairsZone.h"
#	include "ai/trader/ai_trader.h"
#	include "space_restrictor.h"
#	include "physicobject.h"
#	include "item_place_change_enum.h"
#	include "inventory_slots_script_enum.h"
#	include	"ui/UIListItemIconed.h"
#endif