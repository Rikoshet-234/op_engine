#ifndef NightVisionDevice_h
#define NightVisionDevice_h

#include "inventory_item_object.h"
#include "script_export_space.h"
#include "hudsound.h"

class CNightVisionDevice
{
protected:
	bool m_bEnabled;
	bool m_bActive;
	HUD_SOUND				snd_DeviceOn;
	HUD_SOUND				snd_DeviceOff;
	HUD_SOUND				snd_DeviceIdle;
	HUD_SOUND				snd_DeviceBroken;
	shared_str				m_sDeviceSect;	
	shared_str				m_sEffectorSect;
	xr_vector<shared_str>	m_vDisableMaps;
public:
	CNightVisionDevice();
	virtual ~CNightVisionDevice();
	virtual void Load(LPCSTR section);
	void SwitchNightVision();
	void SwitchNightVision(bool vision_state);
	void TurnOn() { SwitchNightVision(true); };
	void TurnOff() { SwitchNightVision(false); };
	void UpdateSwitchNightVision();
	DECLARE_SCRIPT_REGISTER_FUNCTION
};

add_to_type_list(CNightVisionDevice)
#undef script_type_list
#define script_type_list save_type_list(CNightVisionDevice)


class CNightVisionPortable:
	public CNightVisionDevice,
	public CInventoryItemObject
{
public:
					CNightVisionPortable();						
	virtual			~CNightVisionPortable();
	void Load(LPCSTR section) override;
	BOOL	net_Spawn			(CSE_Abstract* DC) override;
	void	net_Destroy			() override;
	void	net_Export			(NET_Packet& P) override;				// export to server
	void	net_Import			(NET_Packet& P) override;				// import from server

	void	OnH_A_Chield		() override;
	void	OnH_B_Independent	(bool just_before_destroy) override;

	void OnMoveToRuck() override;

	void afterDetach() override;
	void	UpdateCL			() override;

	bool	can_be_attached		() const override;

};


#endif