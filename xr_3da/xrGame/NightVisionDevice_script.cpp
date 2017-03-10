#include "pch_script.h"
#include "NightVisionDevice.h"

using namespace luabind;

void CNightVisionDevice::script_register(lua_State *L)
{
	module(L)
		[
			class_<CNightVisionDevice>("CNightVisionDevice")
			// работа с ПНВ
			.def_readwrite("enabled"	,		&CNightVisionDevice::m_bEnabled) // для контроля включаемости
			.def_readonly("active"		,		&CNightVisionDevice::m_bActive)
			.def("turn_on"				,		&CNightVisionDevice::TurnOn)
			.def("turn_off"				,		&CNightVisionDevice::TurnOff)
			.def("switch"				,		static_cast<void (CNightVisionDevice::*)()>(&CNightVisionDevice::SwitchNightVision))

		];

}