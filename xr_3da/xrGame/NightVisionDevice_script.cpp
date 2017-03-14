#include "pch_script.h"
#include "NightVisionDevice.h"

using namespace luabind;
LPCSTR get_section(CNightVisionDevice *pnv)
{
	return pnv->m_sDeviceSect.c_str();
}

void CNightVisionDevice::script_register(lua_State *L)
{
	module(L)
		[
			class_<CNightVisionDevice>("CNightVisionDevice")
			// работа с ПНВ
			.def_readwrite("enabled"	,		&CNightVisionDevice::m_bEnabled) 
			.def_readonly("active"		,		&CNightVisionDevice::m_bActive)
			.def("turn_on"				,		&CNightVisionDevice::TurnOn)
			.def("turn_off"				,		&CNightVisionDevice::TurnOff)
			.def("switch"				,		static_cast<void (CNightVisionDevice::*)()>(&CNightVisionDevice::SwitchNightVision))
			.def("section",&get_section)

		];

}