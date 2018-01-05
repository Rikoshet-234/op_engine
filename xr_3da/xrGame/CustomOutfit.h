#pragma once

#include "inventory_item_object.h"
#include "NightVisionDevice.h"

struct SBoneProtections;

class CCustomOutfit: public CInventoryItemObject {
private:
	typedef	CInventoryItemObject inherited;
public:
									CCustomOutfit		(void);
	virtual							~CCustomOutfit		(void);

	virtual void					Load				(LPCSTR section);
	
	//уменьшенная версия хита, для вызова, когда костюм надет на персонажа
	virtual void					Hit					(SHit *pHDS);

	//коэффициенты на которые домножается хит
	//при соответствующем типе воздействия
	//если на персонаже надет костюм
	float							GetHitTypeProtection(SHit *pHDS);
	float							GetDefHitTypeProtection(ALife::EHitType hit_type);
	float							GetDefHitTypeProtectionOriginal(ALife::EHitType hit_type);

	float							HitThruArmour		(SHit *pHDS);
	//коэффициент на который домножается потеря силы
	//если на персонаже надет костюм
	float							GetPowerLoss		();


	virtual void					OnMoveToSlot		();
	virtual void					OnMoveToRuck		();

	virtual void OnJump() {};
	virtual void OnMove() {};
	virtual void OnSprint() {};

	virtual bool CanJump() { return true; }
	virtual bool CanMove() { return true; }
	virtual bool CanSprint() { return true; }

	virtual void OnCantJump() {};
	virtual void OnCantMove() {};
	virtual void OnCantSprint() {};


protected:
	HitImmunity::HitTypeSVec		m_HitTypeProtection;
	float							m_fPowerLoss;

	shared_str						m_ActorVisual;
	shared_str						m_full_icon_name;
	SBoneProtections*				m_boneProtection;	
	u32								m_ef_equipment_type;
	CNightVisionDevice*				m_cNightVisionDevice;

public:
	float							m_additional_weight;
	float							m_additional_weight2;
	float					        m_fHealthRestoreSpeed;
	float 					        m_fRadiationRestoreSpeed;
	float 					        m_fSatietyRestoreSpeed;
	float					        m_fPowerRestoreSpeed;
	float					        m_fBleedingRestoreSpeed; 
	CNightVisionDevice*				GetNightVisionDevice() const { return m_cNightVisionDevice;};
	
	virtual u32						ef_equipment_type		() const;
	virtual	BOOL					BonePassBullet			(int boneID);
	const shared_str&				GetFullIconName			() const	{return m_full_icon_name;};

	virtual void			net_Export			(NET_Packet& P);
	virtual void			net_Import			(NET_Packet& P);
	void UpdateCL() override;
};
