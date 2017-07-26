#ifndef gbox_h
#define gbox_h
#pragma once

#include "script_export_space.h"
#include "inventory_item_object.h"

class CGBox:public CInventoryItemObject
{
	typedef	CInventoryItemObject	inherited;
	//NET_Packet m_userData;
public:
	CGBox();
	virtual ~CGBox();
	void				Load(LPCSTR section) override;
	BOOL net_Spawn(CSE_Abstract* DC) override;
	void net_Destroy() override;
	void net_Export(NET_Packet& P) override;
	void net_Import(NET_Packet& P) override;
	void net_Relcase(CObject* O) override;

	void SetUserData(NET_Packet* inputPacket);
	void GetUserData(NET_Packet* outputPacket);
	void ClearUserData();
	CGBox *cast_gbox_object() override { return this; };
	/*void	OnH_A_Chield() override;
	void	OnH_B_Chield() override;
	void	OnH_B_Independent(bool just_before_destroy) override;
	void	OnH_A_Independent() override;*/
	DECLARE_SCRIPT_REGISTER_FUNCTION
};

add_to_type_list(CGBox)
#undef script_type_list
#define script_type_list save_type_list(CGBox)

#endif
