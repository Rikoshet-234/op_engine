#ifndef ExoOutfit_h
#define ExoOutfit_h

#include "customoutfit.h"
#include "script_export_space.h"

class CExoOutfit: public CCustomOutfit
{
private:
	typedef	CCustomOutfit inherited;

	float m_fMovingDischarge;
	float m_fSprintDischarge;
	float m_fJumpDischarge;

	u32	dwUpdateSounds_Frame;

	void CExoOutfit::play_sound(ref_sound sound);
	ref_sound sndCantJump;
	ref_sound sndCantSprint;
public:
	CExoOutfit();
	virtual ~CExoOutfit();

	void Load(LPCSTR section) override;


	void OnSprint() override;
	void OnJump() override;
	void OnMove() override;

	bool CanSprint() override;
	bool CanJump() override;
	bool CanMove() override;

	void OnCantJump() override;
	void OnCantSprint() override;

	void UpdateCharge(float value);
	bool isSuitableBattery(shared_str batterySection);
	bool BatteryAccepted() const;
	void RemoveFromBatterySlot(bool spawn = true);
	void PutToBatterySlot(CInventoryItem* item);
	void PutToBatterySlot(shared_str section, float chargeValue);

	//void OnMoveToRuck() override;
	BOOL net_Spawn(CSE_Abstract* DC) override;
	void net_Export(NET_Packet& P) override;
	void net_Import(NET_Packet& P) override;
	//void	UpdateCL() override;
	void TryToUpdateSE();

	void	OnH_B_Independent(bool just_before_destroy) override;

	xr_vector<shared_str> batterySections;
	shared_str m_sCurrentBattery;
	float m_fCurrentCharge;

	bool isBatteryPresent() const { return m_sCurrentBattery.size() > 0; }

	virtual CExoOutfit* cast_exo_object() { return this; }

	DECLARE_SCRIPT_REGISTER_FUNCTION
};

add_to_type_list(CExoOutfit)
#undef script_type_list
#define script_type_list save_type_list(CExoOutfit)
#endif