#include "pch_script.h"
#include "gbox.h"
#include "xrServer_Objects_ALife_Items.h"
#include "ai_space.h"
#include "alife_simulator.h"
#include "alife_object_registry.h"

CGBox::CGBox()
{
	m_class_name = get_class_name<CGBox>(this);
}

CGBox::~CGBox()
{
	
}

void CGBox::Load(LPCSTR section)
{
	inherited::Load(section);
}

BOOL CGBox::net_Spawn(CSE_Abstract* DC)
{
	CSE_ALifeItemGameBox *gbox = smart_cast<CSE_ALifeItemGameBox*>(DC);
	if (!inherited::net_Spawn(DC))
		return FALSE;
	return TRUE;
}

void CGBox::net_Destroy()
{
	inherited::net_Destroy();
}

void CGBox::net_Export(NET_Packet& P)
{
	inherited::net_Export(P);
}

void CGBox::net_Import(NET_Packet& P)
{
	inherited::net_Import(P);
}

void CGBox::net_Relcase(CObject* O)
{
	inherited::net_Relcase(O);
}

void CGBox::SetUserData(NET_Packet* inputPacket)
{
	CSE_ALifeDynamicObject* se_obj = ai().alife().objects().object(ID());
	if (!se_obj)
	{
		Msg("! WARNING cannot find server entry for [%s]. data not stored!",Name());
		return;
	}
	CSE_ALifeItemGameBox* se_gbox = smart_cast<CSE_ALifeItemGameBox*>(se_obj);
	if (!se_gbox)
	{
		Msg("! WARNING server_entry is not CSE_ALifeItemGameBox for [%s]. data not stored!",Name());
		return;
	}
	se_gbox->set_user_data(inputPacket);
}

void CGBox::ClearUserData()
{
	CSE_ALifeDynamicObject* se_obj = ai().alife().objects().object(ID());
	if (!se_obj)
	{
		Msg("! WARNING cannot find server entry for [%s]. data not retreive!", Name());
		return;
	}
	CSE_ALifeItemGameBox* se_gbox = smart_cast<CSE_ALifeItemGameBox*>(se_obj);
	if (!se_gbox)
	{
		Msg("! WARNING server_entry is not CSE_ALifeItemGameBox for [%s]. data not retreive!", Name());
		return;
	}
	se_gbox->clear_user_data();
}

void CGBox::GetUserData(NET_Packet* outputPacket)
{
	CSE_ALifeDynamicObject* se_obj = ai().alife().objects().object(ID());
	if (!se_obj)
	{
		Msg("! WARNING cannot find server entry for [%s]. data not retreive!", Name());
		return;
	}
	CSE_ALifeItemGameBox* se_gbox = smart_cast<CSE_ALifeItemGameBox*>(se_obj);
	if (!se_gbox)
	{
		Msg("! WARNING server_entry is not CSE_ALifeItemGameBox for [%s]. data not retreive!", Name());
		return;
	}
	se_gbox->get_user_data(outputPacket);
}

//void CGBox::OnH_A_Chield()
//{
//	Msg("CGBox::OnH_A_Chield");
//	inherited::OnH_A_Chield();
//}
//
//void CGBox::OnH_B_Chield()
//{
//	Msg("CGBox::OnH_B_Chield");
//	inherited::OnH_B_Chield();
//}
//
//void CGBox::OnH_B_Independent(bool just_before_destroy)
//{
//	Msg("CGBox::OnH_B_Independent");
//	inherited::OnH_B_Independent(just_before_destroy);
//}
//
//void CGBox::OnH_A_Independent()
//{
//	Msg("CGBox::OnH_A_Independent");
//	inherited::OnH_A_Independent();
//}

using namespace luabind;

void CGBox::script_register(lua_State *L)
{
	module(L)
		[
			class_<CGBox, CGameObject>("CGBox")
			.def(constructor<>())
			.def("set_user_data", &CGBox::SetUserData)
			.def("get_user_data", &CGBox::GetUserData)
		];
}
