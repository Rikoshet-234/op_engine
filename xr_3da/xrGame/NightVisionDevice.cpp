#include "stdafx.h"
#include "NightVisionDevice.h"
#include "xrServer_Objects_ALife_Items.h"
#include "Actor.h"
#include "inventory.h"
#include "ai_sounds.h"
#include "../effectorPP.h"
#include "CameraEffector.h"
#include "ActorEffector.h"

CNightVisionDevice::CNightVisionDevice()
{
	m_bEnabled=true;
	m_bActive=false;
}

CNightVisionDevice::~CNightVisionDevice()
{
	HUD_SOUND::DestroySound(snd_DeviceOn);
	HUD_SOUND::DestroySound(snd_DeviceOff);
	HUD_SOUND::DestroySound(snd_DeviceIdle);
	HUD_SOUND::DestroySound(snd_DeviceBroken);
}

inline void loadSoundIfExists(CInifile* ps,shared_str section,LPCSTR sound_name,HUD_SOUND & sound)
{
	if (ps->line_exist(section,sound_name))
		HUD_SOUND::LoadSound(section.c_str(),sound_name,sound,SOUND_TYPE_ITEM_USING);
}

void CNightVisionDevice::Load(LPCSTR section)
{
	R_ASSERT2 (pSettings->section_exist(section), section);
	m_sDeviceSect=section;
	m_sEffectorSect = pSettings->r_string(m_sDeviceSect, "night_vision_effector");
	loadSoundIfExists(pSettings,m_sDeviceSect,"snd_night_vision_on",snd_DeviceOn);
	loadSoundIfExists(pSettings,m_sDeviceSect,"snd_night_vision_off",snd_DeviceOff);
	loadSoundIfExists(pSettings,m_sDeviceSect,"snd_night_vision_idle",snd_DeviceIdle);
	loadSoundIfExists(pSettings,m_sDeviceSect,"snd_night_vision_broken",snd_DeviceBroken);
	if (pSettings->line_exist(m_sDeviceSect,"disabled_maps"))
	{
		LPCSTR disabled_maps	= pSettings->r_string(m_sDeviceSect.c_str(),"disabled_maps");
		u32 cnt					= _GetItemCount(disabled_maps);
		string512				tmp;
		for(u32 i=0; i<cnt; ++i) 
		{
			_GetItem(disabled_maps, i, tmp);
			m_vDisableMaps.push_back(tmp);
		}
	}
}

void CNightVisionDevice::SwitchNightVision()
{
	if (OnClient()) return;
	SwitchNightVision(!m_bActive);	
}

void CNightVisionDevice::SwitchNightVision(bool vision_state)
{
	if (m_bActive == vision_state) 
		return;
	if (vision_state)
		m_bActive = m_bEnabled;
	else
		m_bActive=false;
	if(!g_actor)					return;	
	bool maps_allow			= std::find(m_vDisableMaps.begin(),m_vDisableMaps.end(),Level().name())==m_vDisableMaps.end();
	bool bPlaySoundFirstPerson = (g_actor == Level().CurrentViewEntity());
	if (!m_bEnabled || (m_sEffectorSect.size() && !maps_allow)) {
		HUD_SOUND::PlaySound(snd_DeviceBroken, g_actor->Position(), g_actor, bPlaySoundFirstPerson);
		return;
	}
	if(m_bActive) {
		CEffectorPP* pp = g_actor->Cameras().GetPPEffector(static_cast<EEffectorPPType>(effNightvision));
		if(!pp) 
		{
			if (m_sEffectorSect.size()>0)
			{
				AddEffector(g_actor,effNightvision, m_sEffectorSect);
				HUD_SOUND::PlaySound(snd_DeviceOn, g_actor->Position(), g_actor, bPlaySoundFirstPerson);
				HUD_SOUND::PlaySound(snd_DeviceIdle, g_actor->Position(), g_actor, bPlaySoundFirstPerson, true);
			}
		}
	} else {
		CEffectorPP* pp = g_actor->Cameras().GetPPEffector(static_cast<EEffectorPPType>(effNightvision));
		if(pp){
			pp->Stop			(1.0f);
			HUD_SOUND::PlaySound(snd_DeviceOff, g_actor->Position(), g_actor, bPlaySoundFirstPerson);
			HUD_SOUND::StopSound(snd_DeviceIdle);
		}
	}

}

void CNightVisionDevice::UpdateSwitchNightVision()
{
	if (!m_bEnabled)
	{
		if (m_bActive)	TurnOff();
		return;
	}
	if (OnClient()) return;
}


CNightVisionPortable::CNightVisionPortable()
{
}

CNightVisionPortable::~CNightVisionPortable()
{
	
}

void CNightVisionPortable::Load(LPCSTR section)
{
	CInventoryItemObject::Load(section);
	CNightVisionDevice::Load(section);
	R_ASSERT2 (m_slot==PNV_SLOT, section);
}

BOOL CNightVisionPortable::net_Spawn(CSE_Abstract* DC)
{
	CSE_ALifeItemNVDevice *nvd	= smart_cast<CSE_ALifeItemNVDevice*>(DC);
	if (!CInventoryItemObject::net_Spawn(DC))
		return FALSE;
	collidable.model		= xr_new<CCF_Skeleton>	(this);
	m_bEnabled=nvd->m_enabled;
	SwitchNightVision (nvd->m_active);
	return TRUE;
}

void CNightVisionPortable::net_Destroy()
{
	TurnOff();
	CInventoryItemObject::net_Destroy();
}

void CNightVisionPortable::net_Export(NET_Packet& P)
{
	CInventoryItemObject::net_Export(P);
	u8 flags = 0;
	if (m_bEnabled) flags |= CSE_ALifeItemNVDevice::eEnabled;
	if (m_bActive)  flags |= CSE_ALifeItemNVDevice::eActive;
	const CActor *pA = smart_cast<const CActor *>(H_Parent());
	if (pA)
		if (pA->attached(this))
			flags |= CSE_ALifeItemNVDevice::eAttached;
	P.w_u8(flags);
}

void CNightVisionPortable::net_Import(NET_Packet& P)
{
	CInventoryItemObject::net_Import(P);
	u8 flags = P.r_u8();
	m_bEnabled = !!(flags & CSE_ALifeItemNVDevice::eEnabled);
	m_bActive = !!(flags & CSE_ALifeItemNVDevice::eActive);
	if (m_bActive)
		TurnOn();
}

void CNightVisionPortable::OnH_A_Chield()
{
	CInventoryItemObject::OnH_A_Chield();
}

void CNightVisionPortable::OnH_B_Independent(bool just_before_destroy)
{
	TurnOff();
	HUD_SOUND::StopSound(snd_DeviceOn);
	HUD_SOUND::StopSound(snd_DeviceOff);
	HUD_SOUND::StopSound(snd_DeviceIdle);
	HUD_SOUND::StopSound(snd_DeviceBroken);
	CInventoryItemObject::OnH_B_Independent(just_before_destroy);
}

void CNightVisionPortable::OnMoveToRuck()
{
	CInventoryItemObject::OnMoveToRuck();
	TurnOff();
}

void CNightVisionPortable::afterDetach()
{
	CInventoryItemObject::afterDetach();
	TurnOff();
}

void CNightVisionPortable::UpdateCL()
{
	UpdateSwitchNightVision		();
	CInventoryItemObject::UpdateCL();
}

bool CNightVisionPortable::can_be_attached() const
{
	const CActor *pA = smart_cast<const CActor *>(H_Parent());
	if (pA) 
	{
		u32 slot = GetSlot();
		PIItem item = pA->inventory().m_slots[slot].m_pIItem;
		if( static_cast<const CNightVisionPortable*>(smart_cast<CNightVisionPortable*>(item)) == this )
			return true;
		else
			return false;
	}
	return true;

}
