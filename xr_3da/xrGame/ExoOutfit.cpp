#include "stdafx.h"

#include "exooutfit.h"
#include "actor.h"
#include "ai_space.h"
#include "alife_simulator.h"
#include "alife_object_registry.h"
#include "xrServer_Objects_ALife_Items.h"
#include "xrServer_Objects_ALife.h"
#include "ai_object_location.h"
#include "GameObject.h"
#include "HUDManager.h"
#include "ai_sounds.h"

CExoOutfit::CExoOutfit()
{
	m_fMovingDischarge = 0;
	m_fSprintDischarge = 0;
	m_fJumpDischarge = 0;
	m_fCurrentCharge = 0;
	m_sCurrentBattery = nullptr;
}

CExoOutfit::~CExoOutfit() 
{
	sndCantJump.destroy();
	sndCantSprint.destroy();
}

void CExoOutfit::Load(LPCSTR section)
{
	inherited::Load(section);
	m_fMovingDischarge = READ_IF_EXISTS(pSettings, r_float, section, "discharge_moving", .0f);
	m_fSprintDischarge = READ_IF_EXISTS(pSettings, r_float, section, "discharge_sprint", .0f);
	m_fJumpDischarge = READ_IF_EXISTS(pSettings, r_float, section, "discharge_jump", .0f);

	if (pSettings->line_exist(section, "batteries"))
	{
		_SequenceToList(batterySections, pSettings->r_string(section, "batteries"), ',');
		R_ASSERT3(batterySections.size() != 0, "Batteries must be specified for ", section);
	}

	if (pSettings->line_exist(section, "snd_cant_sprint"))
		sndCantSprint.create(pSettings->r_string(section, "snd_cant_sprint"), st_Effect, SOUND_TYPE_WORLD);
	if (pSettings->line_exist(section, "snd_cant_jump"))
		sndCantJump.create(pSettings->r_string(section, "snd_cant_jump"), st_Effect, SOUND_TYPE_WORLD);
}

void CExoOutfit::UpdateCharge(float value)
{
	m_fCurrentCharge += value;
	clamp(m_fCurrentCharge, .0f, 1.0f);
}

bool CExoOutfit::isSuitableBattery(shared_str batterySection)
{
	return std::find(batterySections.begin(), batterySections.end(), batterySection) != batterySections.end();
}

bool CExoOutfit::BatteryAccepted() const
{
	return !batterySections.empty();
}

void CExoOutfit::RemoveFromBatterySlot(bool spawn)
{
	if (isBatteryPresent() && spawn)
	{
		auto parentId = H_Parent()? H_Parent()->ID() : g_actor->ID();
		CSE_Abstract* abstractItem = Level().spawn_item(m_sCurrentBattery.c_str(), g_actor->Position(), g_actor->ai_location().level_vertex_id(), parentId, true);
		CSE_ALifeInventoryItem*	inventoryItem = smart_cast<CSE_ALifeInventoryItem*>(abstractItem);
		if (inventoryItem)
			inventoryItem->m_fCondition = m_fCurrentCharge;
		NET_Packet P;
		abstractItem->Spawn_Write(P, TRUE);
		Level().Send(P, net_flags(TRUE));
		F_entity_Destroy(abstractItem);
	}
	m_fCurrentCharge=0;
	m_sCurrentBattery = nullptr;
	TryToUpdateSE();
}

void CExoOutfit::PutToBatterySlot(CInventoryItem* item)
{
	RemoveFromBatterySlot();
	R_ASSERT2(item != nullptr, "Invalid input item for CExoOutfitBatterySlot::PutToSlot");
	xr_vector<shared_str>::iterator sectIt = std::find(batterySections.begin(), batterySections.end(), item->object().cNameSect());
	R_ASSERT2(sectIt != batterySections.end(), "Unsupported item for CExoOutfitBatterySlot::PutToSlot");
	m_fCurrentCharge = item->GetCondition();
	m_sCurrentBattery = sectIt->c_str();
	TryToUpdateSE();
	NET_Packet packet;
	packet.w_begin(M_EVENT);
	packet.w_u32(Level().timeServer());
	packet.w_u16(GE_DESTROY);
	packet.w_u16(item->object().ID());
	Level().Send(packet, net_flags(TRUE, TRUE));
}

void CExoOutfit::PutToBatterySlot(shared_str section, float chargeValue)
{
	R_ASSERT3(pSettings->section_exist(section), "Item section must be exists in game configs!", section.c_str());
	RemoveFromBatterySlot();
	m_fCurrentCharge = chargeValue;
	m_sCurrentBattery = section;
	TryToUpdateSE();
}

void CExoOutfit::TryToUpdateSE()
{
	CSE_Abstract* se_obj = ai().alife().objects().object(ID(), true);
	if (!se_obj)
	{
		return;
	}
	CSE_ALifeItemExoOutfit* se_exo = smart_cast<CSE_ALifeItemExoOutfit*>(se_obj);
	if (!se_exo)
	{
		Msg("! WARNING server_entry is not CSE_ALifeItemExoOutfit for [%s].", Name());
		return;
	}
	se_exo->m_fCurrentBatteryCharge = m_fCurrentCharge;
	se_exo->m_sCurrentBatterySection = m_sCurrentBattery;
}

void CExoOutfit::OnH_B_Independent(bool just_before_destroy)
{
	inherited::OnH_B_Independent(just_before_destroy);
}

BOOL CExoOutfit::net_Spawn(CSE_Abstract* DC)
{
	if (!inherited::net_Spawn(DC))
		return(FALSE);
	if (BatteryAccepted())
	{
		CSE_Abstract *abstractObject = static_cast<CSE_Abstract*>(DC);
		CSE_ALifeItemExoOutfit *se_exo = smart_cast<CSE_ALifeItemExoOutfit*>(abstractObject);
		R_ASSERT2(se_exo, "Ivalid input object for CExoOutfit::net_Spawn"); 
		m_fCurrentCharge = se_exo->m_fCurrentBatteryCharge;
		m_sCurrentBattery = se_exo->m_sCurrentBatterySection;
	}
	return TRUE;
}

void CExoOutfit::net_Export(NET_Packet& P)
{
	inherited::net_Export(P);
	//P.w_stringZ(m_sCurrentBattery.size()>0 ? m_sCurrentBattery.c_str() : "");
	P.w_float_q8(m_fCurrentCharge, 0.0f, 1.0f);
}

void CExoOutfit::net_Import(NET_Packet& P)
{
	inherited::net_Import(P);
	//P.r_stringZ(m_sCurrentBattery);
	m_fCurrentCharge = P.r_float_q8(0.0f, 1.0f);
}

void CExoOutfit::OnSprint()
{
	if (!(g_actor->get_state()&(mcFall | mcLanding | mcLanding2)))
		UpdateCharge(-(m_fSprintDischarge / m_fCondition));
}

void CExoOutfit::OnJump()
{
	UpdateCharge(-(m_fJumpDischarge / m_fCondition));
}

void CExoOutfit::OnMove()
{
	if (!(g_actor->get_state()&(mcFall| mcLanding | mcLanding2)))
		UpdateCharge(-(m_fMovingDischarge / m_fCondition));
}

bool CExoOutfit::CanSprint()
{
	return !fsimilar(m_fCurrentCharge, 0, EPS);
}

bool CExoOutfit::CanJump()
{
	return !fsimilar(m_fCurrentCharge, 0,EPS);
}

bool CExoOutfit::CanMove()
{
	return true;
}

void CExoOutfit::play_sound(ref_sound sound)
{
	Fvector snd_pos;
	snd_pos.set(0, ACTOR_HEIGHT, 0);
	//if (!sound._feedback())
	sound.play_at_pos(g_actor,snd_pos, sm_2D);
	//	sound.play_at_pos(nullptr, snd_pos, sm_2D);
	//::Sound->play_at_pos(sound, g_actor, g_actor->Position());


}

void CExoOutfit::OnCantSprint()
{
	play_sound(sndCantSprint);
	HUD().GetUI()->AddInfoMessage("cant_exo_sprint");
}

void CExoOutfit::OnCantJump()
{
	play_sound(sndCantJump);
	HUD().GetUI()->AddInfoMessage("cant_exo_jump");
}

	

