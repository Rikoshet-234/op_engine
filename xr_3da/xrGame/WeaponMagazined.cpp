#include "stdafx.h"
#include "hudmanager.h"
#include "WeaponHUD.h"
#include "WeaponMagazined.h"
#include "entity.h"
#include "actor.h"
#include "ParticlesObject.h"
#include "scope.h"
#include "silencer.h"
#include "GrenadeLauncher.h"
#include "inventory.h"
#include "xrserver_objects_alife_items.h"
#include "ActorEffector.h"
#include "EffectorZoomInertion.h"
#include "xr_level_controller.h"
#include "level.h"
#include "object_broker.h"
#include "string_table.h"
#include "game_object_space.h"
#include "GameObject.h"
#include "script_callback_ex.h"
#include "ui/UIXmlInit.h"
#include "script_game_object.h"
#include "ui/UIInventoryWnd.h"
#include "UIGameSP.h"

CWeaponMagazined::CWeaponMagazined(LPCSTR name, ESoundTypes eSoundType) : CWeapon(name)
{
	m_eSoundShow = ESoundTypes(SOUND_TYPE_ITEM_TAKING | eSoundType);
	m_eSoundHide = ESoundTypes(SOUND_TYPE_ITEM_HIDING | eSoundType);
	m_eSoundShot = ESoundTypes(SOUND_TYPE_WEAPON_SHOOTING | eSoundType);
	m_eSoundEmptyClick = ESoundTypes(SOUND_TYPE_WEAPON_EMPTY_CLICKING | eSoundType);
	m_eSoundReload = ESoundTypes(SOUND_TYPE_WEAPON_RECHARGING | eSoundType);
	m_eSoundZoomIn = ESoundTypes(SOUND_TYPE_ITEM_USING | eSoundType);
	m_eSoundZoomOut = ESoundTypes(SOUND_TYPE_ITEM_USING | eSoundType);
	m_eSoundChangeFireMode = ESoundTypes(SOUND_TYPE_ITEM_USING | eSoundType);
	m_eSoundPreviewAmmo = ESoundTypes(SOUND_TYPE_ITEM_USING | eSoundType);
	m_eSoundZoomInc = ESoundTypes(SOUND_TYPE_ITEM_USING | eSoundType);
	m_eSoundZoomDec = ESoundTypes(SOUND_TYPE_ITEM_USING | eSoundType);

	m_pSndShotCurrent = nullptr;
	m_sSilencerFlameParticles = m_sSilencerSmokeParticles = nullptr;

	m_bFireSingleShot = false;
	m_iShotNum = 0;
	m_iQueueSize = WEAPON_ININITE_QUEUE;
	m_bLockType = false;
	m_iCurFireMode = 0;
	m_bforceReloadAfterIdle = false;
	m_bRequredDemandCheck = true;
	m_pSoundAddonProc = nullptr;
}

CWeaponMagazined::~CWeaponMagazined()
{
	// sounds
	HUD_SOUND::DestroySound(sndShow);
	HUD_SOUND::DestroySound(sndHide);
	HUD_SOUND::DestroySound(sndShot);
	HUD_SOUND::DestroySound(sndEmptyClick);
	HUD_SOUND::DestroySound(sndReload);
	HUD_SOUND::DestroySound(sndChangeFireMode);
	HUD_SOUND::DestroySound(sndPreviewAmmo);
	HUD_SOUND::DestroySound	(sndZoomIn);
	HUD_SOUND::DestroySound	(sndZoomOut);
	HUD_SOUND::DestroySound	(sndZoomDec);
	HUD_SOUND::DestroySound	(sndZoomInc);
}


void CWeaponMagazined::StopHUDSounds		()
{
	HUD_SOUND::StopSound(sndShow);
	HUD_SOUND::StopSound(sndHide);
	
	HUD_SOUND::StopSound(sndEmptyClick);
	HUD_SOUND::StopSound(sndReload);

	HUD_SOUND::StopSound(sndShot);
	HUD_SOUND::StopSound(sndChangeFireMode);
	HUD_SOUND::StopSound(sndPreviewAmmo);

	HUD_SOUND::StopSound(sndZoomOut);
	HUD_SOUND::StopSound(sndZoomIn);

	HUD_SOUND::StopSound(sndZoomInc);
	HUD_SOUND::StopSound(sndZoomDec);
//.	if(sndShot.enable && sndShot.snd.feedback)
//.		sndShot.snd.feedback->switch_to_3D();

	inherited::StopHUDSounds();
}

void CWeaponMagazined::net_Destroy()
{
	inherited::net_Destroy();
}
#include "../xrCore/FTimerStat.h"
BOOL CWeaponMagazined::net_Spawn		(CSE_Abstract* DC)
{
	TSP_SCOPED(_, "CWeaponMagazined::net_Spawn", "spawn");
	return inherited::net_Spawn(DC);
}

void CWeaponMagazined::Load(LPCSTR section)
{
	inherited::Load(section);

#pragma region Load sounds
	HUD_SOUND::LoadSound(section, "snd_draw", sndShow, m_eSoundShow);
	HUD_SOUND::LoadSound(section, "snd_holster", sndHide, m_eSoundHide);
	HUD_SOUND::LoadSound(section, "snd_shoot", sndShot, m_eSoundShot);
	HUD_SOUND::LoadSound(section, "snd_empty", sndEmptyClick, m_eSoundEmptyClick);
	HUD_SOUND::LoadSound(section, "snd_reload", sndReload, m_eSoundReload);
	HUD_SOUND::LoadSound(section, "snd_change_firemode", sndChangeFireMode, m_eSoundChangeFireMode);
	HUD_SOUND::LoadSound(section, "snd_preview_ammo", sndPreviewAmmo, m_eSoundPreviewAmmo);

	HUD_SOUND::LoadSound(section, "snd_zoomin", sndZoomIn, m_eSoundZoomIn, false);
	HUD_SOUND::LoadSound(section, "snd_zoomout", sndZoomOut, m_eSoundZoomOut, false);

	HUD_SOUND::LoadSound(section, "snd_zoomdec", sndZoomInc, m_eSoundZoomInc, false);
	HUD_SOUND::LoadSound(section, "snd_zoominc", sndZoomDec, m_eSoundZoomDec, false);
#pragma endregion
	m_pSndShotCurrent = &sndShot;


	// HUD :: Anims
	R_ASSERT(m_pHUD);
#pragma region Load animations
	animGet(mhud.mhud_idle, pSettings->r_string(*hud_sect, "anim_idle"));
	animGet(mhud.mhud_reload, pSettings->r_string(*hud_sect, "anim_reload"));
	animGet(mhud.mhud_show, pSettings->r_string(*hud_sect, "anim_draw"));
	animGet(mhud.mhud_hide, pSettings->r_string(*hud_sect, "anim_holster"));
	animGet(mhud.mhud_shots, pSettings->r_string(*hud_sect, "anim_shoot"));

	LPCSTR animName = "anim_idle_sprint";
	if (pSettings->line_exist(*hud_sect, animName))
		animGet(mhud.mhud_idle_sprint, pSettings->r_string(*hud_sect, animName), *hud_sect, animName);

	animName = "anim_idle_moving";
	if (pSettings->line_exist(*hud_sect, "anim_idle_moving"))
		animGet(mhud.anim_idle_moving, pSettings->r_string(*hud_sect, animName), *hud_sect, animName);
#pragma endregion 

	if (IsZoomEnabled())
		animGet(mhud.mhud_idle_aim, pSettings->r_string(*hud_sect, "anim_idle_aim"), *hud_sect, "anim_idle_aim");


	//звуки и партиклы глушителя, еслит такой есть
	if (m_eSilencerStatus == ALife::eAddonAttachable)
	{
		if (pSettings->line_exist(section, "silencer_flame_particles"))
			m_sSilencerFlameParticles = pSettings->r_string(section, "silencer_flame_particles");
		if (pSettings->line_exist(section, "silencer_smoke_particles"))
			m_sSilencerSmokeParticles = pSettings->r_string(section, "silencer_smoke_particles");
		if (m_sSilencerFlameParticles && !m_sSilencerSmokeParticles)
			m_sSilencerSmokeParticles = m_sSilencerFlameParticles;
		if (!m_sSilencerFlameParticles && m_sSilencerSmokeParticles)
			m_sSilencerFlameParticles = m_sSilencerSmokeParticles;
		HUD_SOUND::LoadSound(section, "snd_silncer_shot", sndSilencerShot, m_eSoundShot);
	}
	//  [7/20/2005]
	if (pSettings->line_exist(section, "dispersion_start"))
		m_iShootEffectorStart = pSettings->r_u8(section, "dispersion_start");
	else
		m_iShootEffectorStart = 0;
	LoadFireModes(section);
}

bool CWeaponMagazined::UseScopeTexture()
{
	return !m_bEmptyScopeTexture;
}

void CWeaponMagazined::LoadFireModes(LPCSTR section)
{
	if (pSettings->line_exist(section, "fire_modes"))
	{
		m_bHasDifferentFireModes = true;
		shared_str FireModesList = pSettings->r_string(section, "fire_modes");
		int ModesCount = _GetItemCount(FireModesList.c_str());
		m_aFireModes.clear();
		for (int i=0; i<ModesCount; i++)
		{
			string16 sItem;
			_GetItem(FireModesList.c_str(), i, sItem);
			int FireMode = atoi(sItem);
			m_aFireModes.push_back(FireMode);			
		}
		m_iCurFireMode = ModesCount - 1;
		m_iPrefferedFireMode = READ_IF_EXISTS(pSettings, r_s16,section,"preffered_fire_mode",-1);
	}
	else
	{
		//m_aFireModes.clear();
		//m_aFireModes.push_back(1);
		//m_iCurFireMode=0;
		SetQueueSize(1);
		m_bHasDifferentFireModes = false;
	}
}
void CWeaponMagazined::FireStart		()
{
	if(IsValid() && !IsMisfire()) 
	{
		if(!IsWorking() || AllowFireWhileWorking())
		{
			if(GetState()==eReload) return;
			if(GetState()==eShowing) return;
			if(GetState()==eHiding) return;
			if(GetState()==eMisfire) return;

			inherited::FireStart();
			
			if (iAmmoElapsed == 0) 
				OnMagazineEmpty();
			else
				SwitchState(eFire);
		}
	} 
	else 
	{
		if(eReload!=GetState() && eMisfire!=GetState()) OnMagazineEmpty();
	}
}

void CWeaponMagazined::FireEnd() 
{
	inherited::FireEnd();

	CActor	*actor = smart_cast<CActor*>(H_Parent());
	if(!iAmmoElapsed && actor && GetState()!=eReload)
		Reload();
}

void CWeaponMagazined::Reload() 
{
	inherited::Reload();

	TryReload();

}

bool CWeaponMagazined::TryReload() 
{
	if(m_pCurrentInventory) 
	{
		bool isActor = smart_cast<CActor*>(H_Parent()) != nullptr;
		if (isActor && m_bRequredDemandCheck && g_uCommonFlags.is(E_COMMON_FLAGS::gpDemandReload))
		{
			SwitchState(eIdle);
			return false;
		}
		int ammoIndex=m_ammoType;
		if (static_cast<int>(m_ammoType)!=m_iPropousedAmmoType && m_iPropousedAmmoType!=-1)
			ammoIndex=m_iPropousedAmmoType;
		m_pAmmo = smart_cast<CWeaponAmmo*>(m_pCurrentInventory->GetAny(*m_ammoTypes[ammoIndex] ));

		
		if(IsMisfire() && iAmmoElapsed)
		{
			m_bPending = true;
			SwitchState(eReload); 
			return true;
		}

		if(m_pAmmo || unlimited_ammo())  
		{
			m_bPending = true;
			SwitchState(eReload); 
			return true;
		} 
		else 
			if (!m_pAmmo)
			{
				if (!isActor || (isActor && !g_uCommonFlags.is(E_COMMON_FLAGS::gpFixedReload)))
					for (u32 i = 0; i < m_ammoTypes.size(); ++i)
					{
						m_pAmmo = smart_cast<CWeaponAmmo*>(m_pCurrentInventory->GetAny(*m_ammoTypes[i]));
						if (m_pAmmo)
						{
							m_ammoType = i;
							m_iPropousedAmmoType = i;
							m_bPending = true;
							SwitchState(eReload);
							return true;
						}
					}
			}
	}
	
	SwitchState(eIdle);

	return false;
}

bool CWeaponMagazined::IsAmmoAvailable()
{
	if (smart_cast<CWeaponAmmo*>(m_pCurrentInventory->GetAny(*m_ammoTypes[m_ammoType])))
		return	(true);
	else
		for(u32 i = 0; i < m_ammoTypes.size(); ++i)
			if (smart_cast<CWeaponAmmo*>(m_pCurrentInventory->GetAny(*m_ammoTypes[i])))
				return	(true);
	return		(false);
}

bool CWeaponMagazined::ZoomInc()
{
	bool zoomed = inherited::ZoomInc();
	if (zoomed)
		PlaySound(sndZoomInc, get_LastFP());
	return zoomed;
}

bool CWeaponMagazined::ZoomDec()
{
	bool zoomed = inherited::ZoomDec();
	if (zoomed)
		PlaySound(sndZoomDec, get_LastFP());
	return zoomed;
}


void CWeaponMagazined::OnMagazineEmpty() 
{
	//попытка стрелять когда нет патронов
	if(GetState() == eMagEmpty)
	{
		OnEmptyClick			();
		return;
	}

	if( GetNextState() != eMagEmpty && GetNextState() != eReload)
	{
		SwitchState(eMagEmpty);
	}

	inherited::OnMagazineEmpty();
}

void CWeaponMagazined::UnloadMagazine(bool spawn_ammo)
{
	xr_map<LPCSTR, u16> l_ammo;
	
	while(!m_magazine.empty()) 
	{
		CCartridge &l_cartridge = m_magazine.back();
		xr_map<LPCSTR, u16>::iterator l_it;
		for(l_it = l_ammo.begin(); l_ammo.end() != l_it; ++l_it) 
		{
			if(!xr_strcmp(*l_cartridge.m_ammoSect, l_it->first)) 
			{ 
				 ++(l_it->second); 
				 break; 
			}
		}

		if(l_it == l_ammo.end()) l_ammo[*l_cartridge.m_ammoSect] = 1;
		m_magazine.pop_back(); 
		--iAmmoElapsed;
	}

	VERIFY((u32)iAmmoElapsed == m_magazine.size());
	
	if (!spawn_ammo)
		return;

	xr_map<LPCSTR, u16>::iterator l_it;
	for(l_it = l_ammo.begin(); l_ammo.end() != l_it; ++l_it) 
	{
		if (m_pCurrentInventory)
		{
			CWeaponAmmo *l_pA = smart_cast<CWeaponAmmo*>(m_pCurrentInventory->GetAny(l_it->first));
			if(l_pA) 
			{
				u16 l_free = l_pA->m_boxSize - l_pA->m_boxCurr;
				l_pA->m_boxCurr = l_pA->m_boxCurr + (l_free < l_it->second ? l_free : l_it->second);
				l_it->second = l_it->second - (l_free < l_it->second ? l_free : l_it->second);
			}
		}
		if(l_it->second && !unlimited_ammo()) SpawnAmmo(l_it->second, l_it->first);
	}
	if (m_pCurrentInventory)
		m_pCurrentInventory->m_bForceRecalcAmmos=true;
}

void CWeaponMagazined::ReloadMagazine() 
{
	m_bforceReloadAfterIdle=false;
	m_dwAmmoCurrentCalcFrame = 0;	

	//устранить осечку при перезарядке
	if(IsMisfire())	bMisfire = false;
	
	//переменная блокирует использование
	//только разных типов патронов
//	static bool l_lockType = false;
	if (!m_bLockType) {
		m_ammoName	= nullptr;
		m_pAmmo		= nullptr;
	}
	
	if (!m_pCurrentInventory) return;

	if(m_set_next_ammoType_on_reload != u32(-1)){		
		m_ammoType						= m_set_next_ammoType_on_reload;
		m_set_next_ammoType_on_reload	= u32(-1);
	}
	
	if(!unlimited_ammo()) 
	{
		//попытаться найти в инвентаре патроны текущего типа 
		m_pAmmo = smart_cast<CWeaponAmmo*>(m_pCurrentInventory->GetAny(*m_ammoTypes[m_ammoType]));
		
		if(!m_pAmmo && !m_bLockType) 
		{
			for(u32 i = 0; i < m_ammoTypes.size(); ++i) 
			{
				//проверить патроны всех подходящих типов
				m_pAmmo = smart_cast<CWeaponAmmo*>(m_pCurrentInventory->GetAny(*m_ammoTypes[i]));
				if(m_pAmmo) 
				{ 
					m_ammoType = i; 
					break; 
				}
			}
		}
	}
	else
		m_ammoType = m_ammoType;


	//нет патронов для перезарядки
	if(!m_pAmmo && !unlimited_ammo() ) return;

	//разрядить магазин, если загружаем патронами другого типа
	if(!m_bLockType && !m_magazine.empty() && 
		(!m_pAmmo || xr_strcmp(m_pAmmo->cNameSect(), *m_magazine.back().m_ammoSect)))
		UnloadMagazine();

	VERIFY((u32)iAmmoElapsed == m_magazine.size());

	if (m_DefaultCartridge.m_LocalAmmoType != m_ammoType)
		m_DefaultCartridge.Load(*m_ammoTypes[m_ammoType], u8(m_ammoType));
	CCartridge l_cartridge = m_DefaultCartridge;
	while(iAmmoElapsed < iMagazineSize)
	{
		if (!unlimited_ammo())
		{
			if (!m_pAmmo->Get(l_cartridge)) break;
		}
		++iAmmoElapsed;
		l_cartridge.m_LocalAmmoType = u8(m_ammoType);
		m_magazine.push_back(l_cartridge);
	}
	m_ammoName = (m_pAmmo) ? m_pAmmo->m_nameShort : nullptr;

	VERIFY((u32)iAmmoElapsed == m_magazine.size());

	//выкинуть коробку патронов, если она пустая
	if(m_pAmmo && !m_pAmmo->m_boxCurr && OnServer()) 
		m_pAmmo->SetDropManual(TRUE);

	if(iMagazineSize > iAmmoElapsed) 
	{ 
		m_bLockType = true; 
		ReloadMagazine(); 
		m_bLockType = false; 
	}
	if (m_pCurrentInventory)
		m_pCurrentInventory->m_bForceRecalcAmmos=true;
	VERIFY((u32)iAmmoElapsed == m_magazine.size());
}

void CWeaponMagazined::OnStateSwitch	(u32 S)
{
	inherited::OnStateSwitch(S);
	switch (S)
	{
	case eProcessScope:
		{
			if (m_pSoundAddonProc && m_pSoundAddonProc->_handle() && !!m_pSoundAddonProc->_feedback())
				SwitchState(eProcessScope);
			else
			{
				switch (m_sub_state)
				{
					case eSubStateDetachScopeProcess:
						{
							Detach(m_sScopeName.c_str(), true);
							m_sub_state = eSubStateDetachScopeEnd;
						}
						break;
					case eSubStateAttachScopeProcess:
						{
							Attach(g_actor->inventory().Get(m_sScopeName,true),true);
							m_sub_state = eSubStateAttachScopeEnd;
						}
						break;
					default:
						m_sub_state = eSubstateReloadBegin;
						break;
				}
				m_bPending = false;
				m_pSoundAddonProc = nullptr;
				SwitchState(eShowing);
			}
		}
		break;
	case eDetachScope:
		{
			m_sub_state = eSubStateDetachScopeStart;
			SwitchState(eHiding);
		}
		break;
	case eAttachScope:
		{
			if (g_actor && g_actor->inventory().Get(m_sScopeName, true))
			{
				m_sub_state = eSubStateAttachScopeStart;
				SwitchState(eHiding);
				break;
			}
			SwitchState(eIdle);
		}
		break;
	case eIdle:
		switch2_Idle	();
		break;
	case eFire:
		switch2_Fire	();
		break;
	case eFire2:
		switch2_Fire2	();
		break;
	case eMisfire:
		if(smart_cast<CActor*>(this->H_Parent()) && (Level().CurrentViewEntity()==H_Parent()) )
			HUD().GetUI()->AddInfoMessage("gun_jammed");
		break;
	case eMagEmpty:
		switch2_Empty	();
		break;
	case eReload:
		switch2_Reload	();
		break;
	case eShowing:
		switch2_Showing	();
		break;
	case eHiding:
		switch2_Hiding	();
		break;
	case eHidden:
		switch2_Hidden	();
		break;
	}
}

void CWeaponMagazined::UpdateCL			()
{
	inherited::UpdateCL	();
	float dt = Device.fTimeDelta;

	

	//когда происходит апдейт состояния оружия
	//ничего другого не делать
	if(GetNextState() == GetState())
	{
		switch (GetState())
		{
		case eShowing:
		case eHiding:
		case eReload:
		case eIdle:
			fTime			-=	dt;
			if (fTime<0)	
				fTime = 0;
			break;
		case eFire:			
			if(iAmmoElapsed>0)
				state_Fire		(dt);
			
			if(fTime<=0)
			{
				if(iAmmoElapsed == 0)
					OnMagazineEmpty();
				StopShooting();
			}
			else
			{
				fTime			-=	dt;
			}

			break;
		case eMisfire:		state_Misfire	(dt);	break;
		case eMagEmpty:		state_MagEmpty	(dt);	break;
		case eHidden:		break;
		}
	}

	UpdateSounds		();
}

void CWeaponMagazined::UpdateSounds()
{
	if (Device.dwFrame == dwUpdateSounds_Frame)
		return;

	dwUpdateSounds_Frame = Device.dwFrame;

	// ref_sound positions
	if (sndShow.playing())	sndShow.set_position(get_LastFP());
	if (sndHide.playing())	sndHide.set_position(get_LastFP());
	if (sndShot.playing()) sndShot.set_position(get_LastFP());
	if (sndReload.playing()) sndReload.set_position(get_LastFP());
	if (sndEmptyClick.playing())	sndEmptyClick.set_position(get_LastFP());
	if (sndChangeFireMode.playing())	sndChangeFireMode.set_position(get_LastFP());
	if (sndPreviewAmmo.playing())	sndPreviewAmmo.set_position(get_LastFP());

	if (sndZoomIn.playing()) sndZoomIn.set_position(get_LastFP());
	if (sndZoomOut.playing())	sndZoomOut.set_position(get_LastFP());
	if (sndZoomInc.playing()) sndZoomInc.set_position(get_LastFP());
	if (sndZoomDec.playing())	sndZoomDec.set_position(get_LastFP());
}

LPCSTR bstr(bool value)
{
	return value ? "true" : "false";
}

void CWeaponMagazined::state_Fire	(float dt)
{
	VERIFY(fTimeToFire>0.f);
	Fvector					p1, d; 
	p1.set(get_LastFP());
	d.set(get_LastFD());

	if (!H_Parent()) return;

	CInventoryOwner* io		= smart_cast<CInventoryOwner*>(H_Parent());
	if(nullptr == io->inventory().ActiveItem())
	{
			Log("current_state", GetState() );
			Log("next_state", GetNextState());
			Log("state_time", m_dwStateTime);
			Log("item_sect", cNameSect().c_str());
			Log("H_Parent", H_Parent()->cNameSect().c_str());
	}


	smart_cast<CEntity*>	(H_Parent())->g_fireParams	(this, p1,d);
	if (m_iShotNum == 0)
	{
		m_vStartPos = p1;
		m_vStartDir = d;
	};
		
	VERIFY(!m_magazine.empty());
	//Msg("%d && %d && (%d || %d) && (%d || %d)", !m_magazine.empty(), fTime<=0, IsWorking(), m_bFireSingleShot, m_iQueueSize < 0, m_iShotNum < m_iQueueSize);
	//Msg("fTime[%f] IsWorking[%s] m_bFireSingleShot[%s] m_iQueueSize[%i]",fTime, bstr(!!IsWorking()), bstr(m_bFireSingleShot),m_iQueueSize);
	while (!m_magazine.empty() && fTime<=0 && (IsWorking() || m_bFireSingleShot) && (m_iQueueSize < 0 || m_iShotNum < m_iQueueSize))
	{
		m_bFireSingleShot = false;

		VERIFY(fTimeToFire>0.f);
		fTime			+=	fTimeToFire;
		
		++m_iShotNum;
		
		OnShot			();
		static int i = 0;
		if (i||m_iShotNum>m_iShootEffectorStart)
		{
			FireWeaponEvent(GameObject::ECallbackType::eOnActorWeaponFire, GameObject::ECallbackType::eOnNPCWeaponFire);	
			FireTrace		(p1,d);
		}
		else
			FireTrace		(m_vStartPos, m_vStartDir);
	}
	if(m_iShotNum == m_iQueueSize)
		m_bStopedAfterQueueFired = true;


	UpdateSounds			();
}

void CWeaponMagazined::FireWeaponEvent(GameObject::ECallbackType actorCallbackType, GameObject::ECallbackType npcCallbackType)
{
	xr_string ammoType;
	if (GetAmmoElapsed()==0 || m_magazine.empty())
		ammoType=*m_ammoTypes[m_ammoType];
	else
		ammoType = *m_ammoTypes[m_magazine.back().m_LocalAmmoType];
	if (g_actor)
	{
		if (smart_cast<CActor*>(H_Parent()))  // actor
		{
			Actor()->callback(actorCallbackType)(
				lua_game_object(),  // The weapon as a game object.
				ammoType.c_str()   // The caliber of the weapon.
			);
		}
		else 
		{
			if (auto npc=smart_cast<CEntityAlive*>(H_Parent())) //npc
				npc->callback(npcCallbackType)(
				lua_game_object(),                                              // The weapon itself.
				ammoType.c_str()                                               // The caliber of the weapon.
			);
		}
	}
}


void CWeaponMagazined::state_Misfire	(float /**dt/**/)
{
	OnEmptyClick			();
	SwitchState				(eIdle);
	
	bMisfire				= true;

	UpdateSounds			();
}

void CWeaponMagazined::state_MagEmpty	(float dt)
{
}

void CWeaponMagazined::SetDefaults	()
{
	CWeapon::SetDefaults		();
}


void CWeaponMagazined::OnShot		()
{
	// Sound
	PlaySound			(*m_pSndShotCurrent,get_LastFP());

	// Camera	
	AddShotEffector		();

	// Animation
	PlayAnimShoot		();
	
	// Shell Drop
	Fvector vel; 
	PHGetLinearVell(vel);
	OnShellDrop					(get_LastSP(), vel);
	
	// Огонь из ствола
	StartFlameParticles	();

	//дым из ствола
	ForceUpdateFireParticles	();
	StartSmokeParticles			(get_LastFP(), vel);
}


void CWeaponMagazined::OnEmptyClick	()
{
	PlaySound	(sndEmptyClick,get_LastFP());
}

void CWeaponMagazined::OnAnimationEnd(u32 state) 
{
	switch(state) 
	{
		case eReload:	ReloadMagazine();	SwitchState(eIdle);	break;	// End of reload animation
		case eHiding:	
			{
				bool defaultHidding = false;
				CUIInventoryWnd::eInventorySndAction actSoundId= CUIInventoryWnd::eInventorySndAction::eInvSndMax;
				switch (m_sub_state)
				{
					case eSubStateDetachScopeStart:
						{
							m_sub_state = eSubStateDetachScopeProcess;
							actSoundId = CUIInventoryWnd::eInventorySndAction::eInvDetachAddon;
						}
						break;
					case eSubStateAttachScopeStart:
						{
							m_sub_state = eSubStateAttachScopeProcess;
							actSoundId = CUIInventoryWnd::eInventorySndAction::eInvAttachAddon;
						}
						break;
					default:
						defaultHidding = true;
						break;
				}
				if (defaultHidding)
					SwitchState(eHidden);
				else
				{
					m_bPending = true;
					switch2_Hidden();
					if (actSoundId != CUIInventoryWnd::eInventorySndAction::eInvSndMax)
					{
						CUIGameSP* pGameSP = smart_cast<CUIGameSP*>(HUD().GetUI()->UIGame());
						m_pSoundAddonProc = pGameSP->InventoryMenu->GetSound(actSoundId);
						if (m_pSoundAddonProc && m_pSoundAddonProc->_handle())
							m_pSoundAddonProc->play(this, sm_2D);
					}
					SwitchState(eProcessScope);
				}
			}
			break;	// End of Hide
		case eShowing:	
		{
			switch (m_sub_state)
			{
				case eSubStateDetachScopeEnd:
				case eSubStateAttachScopeEnd:
				case eSubStateAttachGLEnd:
				case eSubStateDetachGLEnd:
					m_sub_state = eSubstateReloadBegin;
					break;
				default: break;
			}
			SwitchState(eIdle);
		}break;	// End of Show
		case eIdle:		switch2_Idle();			break;  // Keep showing idle
	}
	inherited::OnAnimationEnd(state);
}
void CWeaponMagazined::switch2_Idle	()
{
	m_bPending = false;
	PlayAnimIdle();
	if (GetForceReloadFlag())
	{
		SetForceReloadFlag(false);
		SwitchState(eReload);
	}
}

#ifdef DEBUG
#include "ai/stalker/ai_stalker.h"
#include "object_handler_planner.h"
#endif
void CWeaponMagazined::switch2_Fire	()
{
	CInventoryOwner* io		= smart_cast<CInventoryOwner*>(H_Parent());
	CInventoryItem* ii		= smart_cast<CInventoryItem*>(this);
#ifdef DEBUG
	if (!io)
		Msg(make_string("no inventory owner, item %s",*cName()).c_str());
	else
	{
	if (ii != io->inventory().ActiveItem())
		Msg					("! not an active item, item %s, owner %s, active item %s",*cName(),*H_Parent()->cName(),io->inventory().ActiveItem() ? *io->inventory().ActiveItem()->object().cName() : "no_active_item");
		if ( !(io && (ii == io->inventory().ActiveItem())) ) 
		{
			CAI_Stalker			*stalker = smart_cast<CAI_Stalker*>(H_Parent());
			if (stalker) {
				stalker->planner().show						();
				stalker->planner().show_current_world_state	();
				stalker->planner().show_target_world_state	();
			}
		}
	}
#else
	if (!io)
		return;
#endif // DEBUG

//
//	VERIFY2(
//		io && (ii == io->inventory().ActiveItem()),
//		make_string(
//			"item[%s], parent[%s]",
//			*cName(),
//			H_Parent() ? *H_Parent()->cName() : "no_parent"
//		)
//	);

	m_bStopedAfterQueueFired = false;
	m_bFireSingleShot = true;
	m_iShotNum = 0;

	if((OnClient() || Level().IsDemoPlay())&& !IsWorking())
		FireStart();

/*	if(SingleShotMode())
	{
		m_bFireSingleShot = true;
		bWorking = false;
	}*/
}
void CWeaponMagazined::switch2_Empty()
{
	OnZoomOut();
	
	if(!TryReload())
	{
		OnEmptyClick();
	}
	else
	{
		inherited::FireEnd();
	}
}
void CWeaponMagazined::PlayReloadSound()
{
	PlaySound	(sndReload,get_LastFP());
}

void CWeaponMagazined::switch2_Reload()
{
	CWeapon::FireEnd();

	PlayReloadSound	();
	PlayAnimReload	();
	m_bPending = true;
}
void CWeaponMagazined::switch2_Hiding()
{
	CWeapon::FireEnd();
	
	PlaySound	(sndHide,get_LastFP());

	PlayAnimHide();
	m_bPending = true;
}

void CWeaponMagazined::switch2_Hidden()
{
	CWeapon::FireEnd();

	if (m_pHUD) m_pHUD->StopCurrentAnimWithoutCallback();

	signal_HideComplete		();
	RemoveShotEffector		();
}
void CWeaponMagazined::switch2_Showing()
{
	PlaySound	(sndShow,get_LastFP());

	m_bPending = true;
	PlayAnimShow();
}

bool CWeaponMagazined::Action(s32 cmd, u32 flags) 
{
	if(inherited::Action(cmd, flags)) return true;
	
	//если оружие чем-то занято, то ничего не делать
	if(IsPending()) return false;
	
	switch(cmd) 
	{
	case kWPN_RELOAD:
		{
			if(flags&CMD_START) 
				if (iAmmoElapsed < iMagazineSize || IsMisfire() || m_ammoType != static_cast<u32>(m_iPropousedAmmoType))
				{
					m_bRequredDemandCheck = false;
					switch2_Empty();//Reload();
					m_bRequredDemandCheck = true;
				}
		} 
		return true;
	case kWPN_FIREMODE_PREV:
		{
			if(flags&CMD_START) 
			{
				OnPrevFireMode();
				return true;
			};
		}break;
	case kWPN_FIREMODE_NEXT:
		{
			if(flags&CMD_START) 
			{
				OnNextFireMode();
				return true;
			};
		}break;
	}
	return false;
}

bool CWeaponMagazined::CanAttach(PIItem pIItem)
{
	CScope*				pScope				= smart_cast<CScope*>(pIItem);
	CSilencer*			pSilencer			= smart_cast<CSilencer*>(pIItem);
	CGrenadeLauncher*	pGrenadeLauncher	= smart_cast<CGrenadeLauncher*>(pIItem);

	if(			pScope &&
				 m_eScopeStatus == ALife::eAddonAttachable &&
				(m_flagsAddOnState&CSE_ALifeItemWeapon::eWeaponAddonScope) == 0 &&
				(m_sScopeName == pIItem->object().cNameSect()) )
	   return true;
	else if(	pSilencer &&
				m_eSilencerStatus == ALife::eAddonAttachable &&
				(m_flagsAddOnState&CSE_ALifeItemWeapon::eWeaponAddonSilencer) == 0 &&
				(m_sSilencerName == pIItem->object().cNameSect()) )
	   return true;
	else if (	pGrenadeLauncher &&
				m_eGrenadeLauncherStatus == ALife::eAddonAttachable &&
				(m_flagsAddOnState&CSE_ALifeItemWeapon::eWeaponAddonGrenadeLauncher) == 0 &&
				(m_sGrenadeLauncherName  == pIItem->object().cNameSect()) )
		return true;
	else
		return inherited::CanAttach(pIItem);
}

bool CWeaponMagazined::CanDetach(const char* item_section_name)
{
	if( m_eScopeStatus == CSE_ALifeItemWeapon::eAddonAttachable &&
	   0 != (m_flagsAddOnState&CSE_ALifeItemWeapon::eWeaponAddonScope) &&
	   (m_sScopeName	== item_section_name))
	   return true;
	else if(m_eSilencerStatus == CSE_ALifeItemWeapon::eAddonAttachable &&
	   0 != (m_flagsAddOnState&CSE_ALifeItemWeapon::eWeaponAddonSilencer) &&
	   (m_sSilencerName == item_section_name))
	   return true;
	else if(m_eGrenadeLauncherStatus == CSE_ALifeItemWeapon::eAddonAttachable &&
	   0 != (m_flagsAddOnState&CSE_ALifeItemWeapon::eWeaponAddonGrenadeLauncher) &&
	   (m_sGrenadeLauncherName == item_section_name))
	   return true;
	else
		return inherited::CanDetach(item_section_name);
}

bool CWeaponMagazined::CanLoadAmmo(CWeaponAmmo* pAmmo,bool checkFullMagazine)
{
	if (!pAmmo) return false;
	if (checkFullMagazine)
	{
		xr_string currentAmmoType;
		if (GetAmmoElapsed()==0 || m_magazine.empty())
			currentAmmoType=*m_ammoTypes[m_ammoType];
		else
			currentAmmoType = *m_ammoTypes[m_magazine.back().m_LocalAmmoType];
		if ((xr_strcmp(currentAmmoType.c_str(),pAmmo->cNameSect().c_str())==0) && iAmmoElapsed==iMagazineSize)
			return false;
	}
	return std::find(m_ammoTypes.begin(),m_ammoTypes.end(),pAmmo->cNameSect()) != m_ammoTypes.end();
}

void CWeaponMagazined::LoadAmmo(CWeaponAmmo* pAmmo)
{
	if (!pAmmo)
	{
		Msg("! WARNING not possible to load null ammo in magazine for [%s]",Name());
		return;
	}
	xr_string currentAmmoType;
	if (GetAmmoElapsed()==0 || m_magazine.empty())
		currentAmmoType=*m_ammoTypes[m_ammoType];
	else
		currentAmmoType = *m_ammoTypes[m_magazine.back().m_LocalAmmoType];
	if ((xr_strcmp(currentAmmoType.c_str(),pAmmo->cNameSect().c_str())==0) && iAmmoElapsed==iMagazineSize)
		return;
	xr_vector<shared_str>::iterator am_it=std::find(m_ammoTypes.begin(),m_ammoTypes.end(),pAmmo->cNameSect());
	m_set_next_ammoType_on_reload=u32(-1);
	if (am_it!=m_ammoTypes.end())
	{
		m_set_next_ammoType_on_reload=std::distance(m_ammoTypes.begin(),am_it);
		if (m_set_next_ammoType_on_reload!=u32(-1))
		{
			m_bforceReloadAfterIdle=true;
			UnloadMagazine();
		}
	}
}

bool CWeaponMagazined::AttachScopeSection(const char* item_section_name, bool singleAttach)
{
	if (!g_actor)
		return false;
	CInventoryItem* iitem = g_actor->inventory().Get(item_section_name,true);
	if (iitem)//нашли
	{
		CScope*				pScope = smart_cast<CScope*>(iitem);
		if (pScope && m_eScopeStatus == CSE_ALifeItemWeapon::eAddonAttachable && (m_flagsAddOnState&CSE_ALifeItemWeapon::eWeaponAddonScope) == 0)
		{
			m_flagsAddOnState |= CSE_ALifeItemWeapon::eWeaponAddonScope;
			iitem->object().DestroyObject();
			if (singleAttach)//если аттач только одного итема а не всех подряд
			{
				UpdateAddonsVisibility();
				InitAddons();
			}
			return true;
		}
	}
	return false;
}

bool is_fake_scope(LPCSTR section)
{
	if (!section)
		return false;
	if (pSettings->line_exist(section, "scope_texture"))
		return xr_strlen(pSettings->r_string(section, "scope_texture")) == 0;
	return false;
}


bool CWeaponMagazined::Attach(PIItem pIItem, bool b_send_event)
{
	bool result = false;

	CScope*				pScope					= smart_cast<CScope*>(pIItem);
	CSilencer*			pSilencer				= smart_cast<CSilencer*>(pIItem);
	CGrenadeLauncher*	pGrenadeLauncher		= smart_cast<CGrenadeLauncher*>(pIItem);
	
	if(pScope &&
	   m_eScopeStatus == CSE_ALifeItemWeapon::eAddonAttachable &&
	   (m_flagsAddOnState&CSE_ALifeItemWeapon::eWeaponAddonScope) == 0 &&
	   (m_sScopeName == pIItem->object().cNameSect()))
	{
		if (IsGrenadeLauncherAttached() && is_fake_scope(m_sGrenadeLauncherName.c_str()))
		{
			Detach(m_sGrenadeLauncherName.c_str(), true);
		}
		m_flagsAddOnState |= CSE_ALifeItemWeapon::eWeaponAddonScope;
		result = true;
	}
	else if(pSilencer &&
	   m_eSilencerStatus == CSE_ALifeItemWeapon::eAddonAttachable &&
	   (m_flagsAddOnState&CSE_ALifeItemWeapon::eWeaponAddonSilencer) == 0 &&
	   (m_sSilencerName == pIItem->object().cNameSect()))
	{
		m_flagsAddOnState |= CSE_ALifeItemWeapon::eWeaponAddonSilencer;
		result = true;
	}
	else if(pGrenadeLauncher &&
	   m_eGrenadeLauncherStatus == CSE_ALifeItemWeapon::eAddonAttachable &&
	   (m_flagsAddOnState&CSE_ALifeItemWeapon::eWeaponAddonGrenadeLauncher) == 0 &&
	   (m_sGrenadeLauncherName == pIItem->object().cNameSect()))
	{
		m_flagsAddOnState |= CSE_ALifeItemWeapon::eWeaponAddonGrenadeLauncher;
		result = true;
	}

	if(result)
	{
		if (b_send_event && OnServer())
		{
			//уничтожить подсоединенную вещь из инвентаря
//.			pIItem->Drop					();
			pIItem->object().DestroyObject	();
		};

		UpdateAddonsVisibility();
		InitAddons();

		return true;
	}
	else
		return inherited::Attach(pIItem, b_send_event);
}


bool CWeaponMagazined::Detach(const char* item_section_name, bool b_spawn_item)
{
	if(		m_eScopeStatus == CSE_ALifeItemWeapon::eAddonAttachable &&
			0 != (m_flagsAddOnState&CSE_ALifeItemWeapon::eWeaponAddonScope) &&
			(m_sScopeName == item_section_name))
	{
		m_flagsAddOnState &= ~CSE_ALifeItemWeapon::eWeaponAddonScope;
		
		UpdateAddonsVisibility();
		InitAddons();

		return CInventoryItemObject::Detach(item_section_name, b_spawn_item);
	}
	else if(m_eSilencerStatus == CSE_ALifeItemWeapon::eAddonAttachable &&
			0 != (m_flagsAddOnState&CSE_ALifeItemWeapon::eWeaponAddonSilencer) &&
			(m_sSilencerName == item_section_name))
	{
		m_flagsAddOnState &= ~CSE_ALifeItemWeapon::eWeaponAddonSilencer;

		UpdateAddonsVisibility();
		InitAddons();
		return CInventoryItemObject::Detach(item_section_name, b_spawn_item);
	}
	else if(m_eGrenadeLauncherStatus == CSE_ALifeItemWeapon::eAddonAttachable &&
			0 != (m_flagsAddOnState&CSE_ALifeItemWeapon::eWeaponAddonGrenadeLauncher) &&
			(m_sGrenadeLauncherName == item_section_name))
	{
		m_flagsAddOnState &= ~CSE_ALifeItemWeapon::eWeaponAddonGrenadeLauncher;

		UpdateAddonsVisibility();
		InitAddons();
		return CInventoryItemObject::Detach(item_section_name, b_spawn_item);
	}
	else
		return inherited::Detach(item_section_name, b_spawn_item);;
}

shared_str GenScopeTextureName(shared_str startTextureName,shared_str openPostfix,shared_str partPostfix)
{
	string_path	fn;
	shared_str final_scope_texture;
	std::string fixedName=startTextureName.c_str();
	size_t lastdot = fixedName.find_last_of("."); //remove ext if present in filename
	if (lastdot!=std::string::npos)
		fixedName=fixedName.substr(0, lastdot).c_str(); 

	if (partPostfix.size()>0) //remove part prefix if present in filename
	{
		std::string testStr=fixedName.substr(fixedName.length()-partPostfix.size(), partPostfix.size()).c_str(); 
		if (xr_strcmp(testStr.c_str(),partPostfix.c_str())==0)
		{
			fixedName=fixedName.substr(0,fixedName.length()-partPostfix.size());
		}
	}
	if (g_uCommonFlags.test(E_COMMON_FLAGS::gpOpenScope))
	{
		string256 buf;
		sprintf_s(buf,"%s%s%s",fixedName.c_str(),partPostfix.c_str(),openPostfix.c_str());
		if (FS.exist(fn,"$game_textures$",buf,".dds"))
			final_scope_texture=buf;	
		else
		{
			sprintf_s(buf,"%s%s",fixedName.c_str(),partPostfix.c_str());
			if (FS.exist(fn,"$game_textures$",buf,".dds"))
				final_scope_texture=buf;	
		}
	}
	else
	{
		string256 buf;
		sprintf_s(buf,"%s%s",fixedName.c_str(),partPostfix.c_str());
		if (FS.exist(fn,"$game_textures$",buf,".dds"))
			final_scope_texture=buf;				
	}
	return final_scope_texture;
}

void CWeaponMagazined::InitAddons()
{
	//////////////////////////////////////////////////////////////////////////
	// Прицел
	m_fIronSightZoomFactor = READ_IF_EXISTS(pSettings, r_float, cNameSect(), "ironsight_zoom_factor", 50.0f);
	if(IsScopeAttached())
	{
		shared_str scope_tex_name;
		if(m_eScopeStatus == ALife::eAddonAttachable)
		{
			m_sScopeName = pSettings->r_string(cNameSect(), "scope_name");
			m_iScopeX	 = pSettings->r_s32(cNameSect(),"scope_x");
			m_iScopeY	 = pSettings->r_s32(cNameSect(),"scope_y");
			scope_tex_name = pSettings->r_string(*m_sScopeName, "scope_texture");
			m_fScopeZoomFactor = pSettings->r_float	(*m_sScopeName, "scope_zoom_factor");
			m_fScopeZoomStepCount = READ_IF_EXISTS(pSettings, r_float, *m_sScopeName, "scope_zoom_step_count", m_fScopeZoomStepCount==0 ? 1.0f : m_fScopeZoomStepCount);
		}
		else if(m_eScopeStatus == ALife::eAddonPermanent)
		{
			scope_tex_name = pSettings->r_string(cNameSect(), "scope_texture");
			m_fScopeZoomFactor = pSettings->r_float	(cNameSect(), "scope_zoom_factor");
			m_fScopeZoomStepCount = READ_IF_EXISTS(pSettings, r_float, cNameSect(), "scope_zoom_step_count", m_fScopeZoomStepCount==0 ? 1.0f : m_fScopeZoomStepCount);
		}
		m_bEmptyScopeTexture = scope_tex_name.size() == 0;
		if(m_UIScope /*&& (!m_UIScope->GetTextureName() || m_UIScope->GetTextureName().equal(scope_tex_name))*/)
		{
			xr_delete(m_UIScope);
		}
		if (!m_UIScope && !m_bEmptyScopeTexture)
		{
			CUIXml									uiXml;
			uiXml.Init								(CONFIG_PATH, UI_PATH,"scope.xml");

			m_UIScope = xr_new<CUIWindow>();
			CUIXmlInit::InitWindow(uiXml,"wpn_scope",0,m_UIScope);
			CUIWindow* scopeWindow=m_UIScope->FindChild("scope_texture");
			if (scopeWindow)
			{
#pragma region set texture for central part of scene 
				CUIStatic* scopeStatic=smart_cast<CUIStatic*>(scopeWindow);
				shared_str preparedTextureName=GenScopeTextureName(scope_tex_name,"_open","");
				if (xr_strlen(preparedTextureName)==0)
					preparedTextureName=scope_tex_name;
				scopeStatic->InitTexture(preparedTextureName.c_str());
#pragma endregion
#pragma region set texture for dump (left/right) parts of scene 
				CUIWindow* leftScopeWindow=m_UIScope->FindChild("left_texture");//find default left and right parts
				CUIWindow* rightScopeWindow=m_UIScope->FindChild("right_texture");
				if (leftScopeWindow && rightScopeWindow)//part found,maybe widescreen?
				{
					CUIStatic* leftScopeStatic=smart_cast<CUIStatic*>(leftScopeWindow);
					CUIStatic* rightScopeStatic=smart_cast<CUIStatic*>(rightScopeWindow);
					xr_string defTextureLeft=leftScopeStatic->GetApplTextureName();
					shared_str lDumpTexture=GenScopeTextureName(scope_tex_name,"_open","_l");
					if (xr_strlen(lDumpTexture)==0)
					{
						lDumpTexture=GenScopeTextureName(defTextureLeft.c_str(),"_open","_l");
						if (xr_strlen(lDumpTexture)==0)
							lDumpTexture=defTextureLeft.c_str();
					}

					xr_string defTextureRight=rightScopeStatic->GetApplTextureName();
					shared_str rDumpTexture=GenScopeTextureName(scope_tex_name,"_open","_r");
					if (xr_strlen(rDumpTexture)==0)
					{
						rDumpTexture=GenScopeTextureName(defTextureRight.c_str(),"_open","_r");
						if (xr_strlen(rDumpTexture)==0)
							rDumpTexture=defTextureRight.c_str();
					}
					leftScopeStatic->InitTexture(lDumpTexture.c_str());
					rightScopeStatic->InitTexture(rDumpTexture.c_str());
				}
#pragma endregion
			}
			else
				Msg("! ERROR can't find static for scope texture in config!");
		}
		m_fRTZoomFactor=m_fScopeZoomFactor; //начальное значение
	}
	else
	{
		if(m_UIScope) xr_delete(m_UIScope);
		
		if (IsZoomEnabled())
		{
			m_fIronSightZoomFactor = pSettings->r_float(cNameSect(), "scope_zoom_factor");
		}
	}

	if (m_bZoomEnabled && m_pHUD) 
		LoadZoomOffset(*hud_sect, m_bEmptyScopeTexture && IsScopeAttached() ? "scope_" : "");

	if(IsSilencerAttached() && SilencerAttachable())
	{		
		m_sFlameParticlesCurrent = m_sSilencerFlameParticles;
		m_sSmokeParticlesCurrent = m_sSilencerSmokeParticles;
		m_pSndShotCurrent = &sndSilencerShot;


		//сила выстрела
		LoadFireParams	(*cNameSect(), "");

		//подсветка от выстрела
		LoadLights		(*cNameSect(), "silencer_");
		ApplySilencerKoeffs();
	}
	else
	{
		m_sFlameParticlesCurrent = m_sFlameParticles;
		m_sSmokeParticlesCurrent = m_sSmokeParticles;
		m_pSndShotCurrent = &sndShot;

		//сила выстрела
		LoadFireParams	(*cNameSect(), "");
		//подсветка от выстрела
		LoadLights		(*cNameSect(), "");
	}

	inherited::InitAddons();
}

void CWeaponMagazined::ApplySilencerKoeffs	()
{
	float BHPk = 1.0f, BSk = 1.0f;
	float FDB_k = 1.0f, CD_k = 1.0f;
	
	if (pSettings->line_exist(m_sSilencerName, "bullet_hit_power_k"))
	{
		BHPk = pSettings->r_float(m_sSilencerName, "bullet_hit_power_k");
		clamp(BHPk, 0.0f, 1.0f);
	};
	if (pSettings->line_exist(m_sSilencerName, "bullet_speed_k"))
	{
		BSk = pSettings->r_float(m_sSilencerName, "bullet_speed_k");
		clamp(BSk, 0.0f, 1.0f);
	};
	if (pSettings->line_exist(m_sSilencerName, "fire_dispersion_base_k"))
	{
		FDB_k = pSettings->r_float(m_sSilencerName, "fire_dispersion_base_k");
//		clamp(FDB_k, 0.0f, 1.0f);
	};
	if (pSettings->line_exist(m_sSilencerName, "cam_dispersion_k"))
	{
		CD_k = pSettings->r_float(m_sSilencerName, "cam_dispersion_k");
		clamp(CD_k, 0.0f, 1.0f);
	};

	//fHitPower			= fHitPower*BHPk;
	fvHitPower			.mul(BHPk);
	fHitImpulse			*= BSk;
	m_fStartBulletSpeed *= BSk;
	fireDispersionBase	*= FDB_k;
	camDispersion		*= CD_k;
	camDispersionInc	*= CD_k;
}

//виртуальные функции для проигрывания анимации HUD
void CWeaponMagazined::PlayAnimShow()
{
	VERIFY(GetState()==eShowing);
	m_pHUD->animPlay(random_anim(mhud.mhud_show),FALSE,this,GetState());
}

void CWeaponMagazined::PlayAnimHide()
{
	VERIFY(GetState()==eHiding);
	m_pHUD->animPlay (random_anim(mhud.mhud_hide),TRUE,this,GetState());
}


void CWeaponMagazined::PlayAnimReload()
{
	VERIFY(GetState()==eReload);
	m_pHUD->animPlay(random_anim(mhud.mhud_reload),TRUE,this,GetState());
}

bool CWeaponMagazined::TryPlayAnimIdle()
{
	VERIFY(GetState()==eIdle);
	if(!IsZoomed()){
		CActor* pActor = smart_cast<CActor*>(H_Parent());
		if(pActor)
		{
			CEntity::SEntityState st;
			pActor->g_State(st);
			if(st.bSprint)
			{
				//return PlayAnimation(mhud.mhud_idle_sprint,TRUE,"try play [mhud.mhud_idle_sprint]");
				return PlayAnimation(mhud.mhud_idle_sprint,TRUE);
			} 
			else if (pActor->AnyMove())
			{
				//return PlayAnimation(mhud.anim_idle_moving,TRUE,"try play [mhud.anim_idle_moving]");
				return PlayAnimation(mhud.anim_idle_moving,TRUE);					
			}
		}
	}
	return false;
}

void CWeaponMagazined::PlayPreviewAmmoSound()
{
	PlaySound(sndPreviewAmmo, get_LastFP());
}

void CWeaponMagazined::PlayAnimIdle()
{
	MotionSVec* m = nullptr;
	if(IsZoomed())
	{
		m = &mhud.mhud_idle_aim;
	}
	else{
		m = &mhud.mhud_idle;
		if (TryPlayAnimIdle()) return;
	}

	VERIFY(GetState()==eIdle);

	//string256 buf;sprintf_s(buf,"try play [%s]",IsZoomed()?"mhud.mhud_idle_aim":"mhud.mhud_idle");PlayAnimation(*m,TRUE,buf);
	PlayAnimation(*m,TRUE);
}

void CWeaponMagazined::PlayAnimShoot()
{
	VERIFY(GetState()==eFire || GetState()==eFire2);
	m_pHUD->animPlay(random_anim(mhud.mhud_shots), TRUE, this, GetState());
}

void CWeaponMagazined::OnZoomIn			()
{
	inherited::OnZoomIn();

	if(GetState() == eIdle)
		PlayAnimIdle();

	if (H_Parent())
	{
		HUD_SOUND::StopSound(sndZoomOut);
		bool b_hud_mode = (Level().CurrentEntity() == H_Parent());
		HUD_SOUND::PlaySound(sndZoomIn,H_Parent()->Position(), H_Parent(), b_hud_mode);
	}

	CActor* pActor = smart_cast<CActor*>(H_Parent());
	if(pActor)
	{
		CEffectorZoomInertion* S = smart_cast<CEffectorZoomInertion*>	(pActor->Cameras().GetCamEffector(eCEZoom));
		if (!S)	
		{
			S = (CEffectorZoomInertion*)pActor->Cameras().AddCamEffector(xr_new<CEffectorZoomInertion> ());
			S->Init(this);
		};
		S->SetRndSeed(pActor->GetZoomRndSeed());
		R_ASSERT				(S);
	}
}
void CWeaponMagazined::OnZoomOut		()
{
	if(!m_bZoomMode) return;

	inherited::OnZoomOut();

	if(GetState() == eIdle)
		PlayAnimIdle();

	if (H_Parent() && !IsRotatingToZoom())
	{
		HUD_SOUND::StopSound(sndZoomIn);
		bool b_hud_mode = (Level().CurrentEntity() == H_Parent());	
		HUD_SOUND::PlaySound(sndZoomOut, H_Parent()->Position(), H_Parent(), b_hud_mode);
	}
	CActor* pActor = smart_cast<CActor*>(H_Parent());
	if(pActor)
		pActor->Cameras().RemoveCamEffector	(eCEZoom);
}

//переключение режимов стрельбы одиночными и очередями
bool CWeaponMagazined::SwitchMode			()
{
	if(eIdle != GetState() || IsPending()) return false;

	if(SingleShotMode())
		m_iQueueSize = WEAPON_ININITE_QUEUE;
	else
		m_iQueueSize = 1;
	
	PlaySound	(sndEmptyClick, get_LastFP());

	return true;
}
 
void CWeaponMagazined::StartIdleAnim			()
{
	if(IsZoomed())	m_pHUD->animDisplay(mhud.mhud_idle_aim[Random.randI(mhud.mhud_idle_aim.size())], TRUE);
	else			m_pHUD->animDisplay(mhud.mhud_idle[Random.randI(mhud.mhud_idle.size())], TRUE);
}

void CWeaponMagazined::onMovementChanged	(ACTOR_DEFS::EMoveCommand cmd)
{
	switch (cmd)
	{
		case ACTOR_DEFS::mcSprint:
		case ACTOR_DEFS::mcAnyMove: 
			if (GetState()==eIdle) 
				PlayAnimIdle(); 
			break;
		case mcFwd: break;
		case mcBack: break;
		case mcLStrafe: break;
		case mcRStrafe: break;
		case mcCrouch: break;
		case mcAccel: break;
		case mcTurn: break;
		case mcJump: break;
		case mcFall: break;
		case mcLanding: break;
		case mcLanding2: break;
		case mcClimb: break;
		case mcLLookout: break;
		case mcRLookout: break;
		case mcAnyAction: break;
		case mcAnyState: break;
		case mcLookout: break;
		default: break;
	}
}

void	CWeaponMagazined::OnNextFireMode		()
{
	if (!m_bHasDifferentFireModes) return;
	if (GetState() != eIdle) return;
	PlaySound	(sndChangeFireMode,get_LastFP());
	m_iCurFireMode = (m_iCurFireMode+1+m_aFireModes.size()) % m_aFireModes.size();
	SetQueueSize(GetCurrentFireMode());
};

void	CWeaponMagazined::OnPrevFireMode		()
{
	if (!m_bHasDifferentFireModes) return;
	if (GetState() != eIdle) return;
	PlaySound	(sndChangeFireMode,get_LastFP());
	m_iCurFireMode = (m_iCurFireMode-1+m_aFireModes.size()) % m_aFireModes.size();
	SetQueueSize(GetCurrentFireMode());	
};

void	CWeaponMagazined::OnH_A_Chield		()
{
	if (m_bHasDifferentFireModes)
	{
		CActor	*actor = smart_cast<CActor*>(H_Parent());
		if (!actor) SetQueueSize(-1);
		else SetQueueSize(GetCurrentFireMode());
	}
	else
		SetQueueSize(1);
	inherited::OnH_A_Chield();
};

void	CWeaponMagazined::SetQueueSize			(int size)  
{
	m_iQueueSize = size; 
	if (m_iQueueSize == -1)
		strcpy_s(m_sCurFireMode, " (A)");
	else if (m_bHasDifferentFireModes)
			sprintf_s(m_sCurFireMode, " (%d)", m_iQueueSize);
};

float	CWeaponMagazined::GetWeaponDeterioration	()
{
	if (!m_bHasDifferentFireModes || m_iPrefferedFireMode == -1 || u32(GetCurrentFireMode()) <= u32(m_iPrefferedFireMode)) 
		return inherited::GetWeaponDeterioration();
	return m_iShotNum*conditionDecreasePerShot;
};

void CWeaponMagazined::save(NET_Packet &output_packet)
{
	inherited::save	(output_packet);
	save_data		(m_iQueueSize, output_packet);
	save_data		(m_iShotNum, output_packet);
	if (m_iCurFireMode<0 || m_iCurFireMode>10)
		m_iCurFireMode=0;
	save_data		(m_iCurFireMode, output_packet);
}

void CWeaponMagazined::load(IReader &input_packet)
{
	inherited::load	(input_packet);
	load_data		(m_iQueueSize, input_packet);SetQueueSize(m_iQueueSize);
	load_data		(m_iShotNum, input_packet);
	load_data		(m_iCurFireMode, input_packet);
	if (m_iCurFireMode<0 || m_iCurFireMode>10)
		m_iCurFireMode=0;
}

void CWeaponMagazined::net_Export	(NET_Packet& P)
{
	inherited::net_Export (P);
	if (m_iCurFireMode<0 || m_iCurFireMode>10)
		m_iCurFireMode=0;
	P.w_u8(u8(m_iCurFireMode&0x00ff));
}

void CWeaponMagazined::net_Import	(NET_Packet& P)
{
	//	if (Level().IsDemoPlay())
	//		Msg("CWeapon::net_Import [%d]", ID());

	inherited::net_Import (P);

	m_iCurFireMode = P.r_u8();

	if (m_iCurFireMode<0 || m_iCurFireMode>10)
		m_iCurFireMode=0;
	if (m_bHasDifferentFireModes)
		SetQueueSize(GetCurrentFireMode());
	else
		SetQueueSize(1);
}
#include "string_table.h"
u32 CWeaponMagazined::getCurrentAmmoType()
{
	u32 result;
	if(GetAmmoElapsed()==0 || m_magazine.size()==0)
		result	= m_ammoType;
	else
		result	= m_magazine.back().m_LocalAmmoType;
	return result;
}
void CWeaponMagazined::GetBriefInfo(xr_string& str_name, xr_string& icon_sect_name, xr_string& str_count)
{
	int	AE					= GetAmmoElapsed();
	if (m_iPropousedAmmoType==-1)
		m_iPropousedAmmoType	= getCurrentAmmoType();
	icon_sect_name	= *m_ammoTypes[m_iPropousedAmmoType];
	int sectionCount=0;
	int	AC					= GetAmmoCurrentEx(sectionCount);
	string256		sItemName;
	strcpy_s			(sItemName, *CStringTable().translate(pSettings->r_string(icon_sect_name.c_str(), "inv_name_short")));
	if ( HasFireModes() )
		strcat_s(sItemName, GetCurrentFireModeStr());
	else if (GetAmmoMagSize()!=0)
		strcat_s(sItemName, " (1)");
	str_name		= sItemName;
	bool multiAmmo=false;
	CWeaponAmmo *lastFounded=nullptr;
	std::for_each(m_ammoTypes.begin(),m_ammoTypes.end(),[&](shared_str section)
	{
		auto founded=std::find_if(m_pCurrentInventory->m_ruck.begin(),m_pCurrentInventory->m_ruck.end(),
			[&](CInventoryItem* item)
			{
				CWeaponAmmo *l_pAmmo = smart_cast<CWeaponAmmo*>(item);
				if (!l_pAmmo)
					return false;
				return xr_strcmp(l_pAmmo->cNameSect().c_str(),section.c_str())==0;
			});
		if (founded!=m_pCurrentInventory->m_ruck.end())
		{
			CWeaponAmmo *ammo = smart_cast<CWeaponAmmo*>(*founded);
			lastFounded=ammo;
			if (xr_strcmp(icon_sect_name.c_str(),ammo->cNameSect().c_str())!=0)
				multiAmmo=true;
		}
	});
	if (!multiAmmo)
		if (lastFounded && xr_strcmp(*m_ammoTypes[getCurrentAmmoType()],lastFounded->cNameSect())!=0)
			multiAmmo=true;
	if (GetAmmoMagSize()!=0)
		if (!unlimited_ammo())
			if (multiAmmo)
				sprintf_s			(sItemName, "%d/%d/%d",AE,sectionCount,AC - AE);
			else
				sprintf_s			(sItemName, "%d/%d",AE,AC - AE);
		else
			sprintf_s			(sItemName, "%d/--/--",AE);

	str_count				= sItemName;
}
