// File:        UIComboBox_script.cpp
// Description: exports CUIComobBox to LUA environment
// Created:     11.12.2004
// Author:      Serhiy O. Vynnychenko
// Mail:        narrator@gsc-game.kiev.ua
//
// Copyright 2004 GSC Game World
//

#include "pch_script.h"
#include "UIComboBox.h"
#include "UIListBoxItem.h"

using namespace luabind;

#pragma optimize("s",on)
void CUIComboBox::script_register(lua_State *L)
{
	module(L)
	[
		class_<CUIComboBox, CUIWindow>("CUIComboBox")
		.def(						constructor<>())
		.def("Init",				(void (CUIComboBox::*)(float, float, float))   &CUIComboBox::Init)
		.def("Init",				(void (CUIComboBox::*)(float, float, float, float))   &CUIComboBox::Init)		
		.def("SetVertScroll",		&CUIComboBox::SetVertScroll)
		.def("SetListLength",		&CUIComboBox::SetListLength)
		.def("CurrentID",			&CUIComboBox::CurrentID)
		.def("SetCurrentID",		&CUIComboBox::SetItem)
		
//		.def("AddItem",				(void (CUIComboBox::*)(LPCSTR, bool)) CUIComboBox::AddItem)
//		.def("AddItem",				(void (CUIComboBox::*)(LPCSTR)) CUIComboBox::AddItem)
	];
}

void CUIComboBoxCustom::script_register(lua_State *L)
{
	module(L)
	[
		class_<CUIListBoxItem>("CUIListBoxItem")
		.def(						constructor<>()),

		class_<CUIComboBoxCustom, CUIStatic>("CUIComboBoxCustom")
		.def(						constructor<>())
		.def("SetListLength",		&CUIComboBoxCustom::SetListLength)
		.def("SetOwner",			&CUIComboBoxCustom::SetOwner)
		//.def("AddItem",				static_cast<CUIListBoxItem* (CUIComboBoxCustom::*)(LPCSTR, int)>(&CUIComboBoxCustom::AddItem))
	];
}

