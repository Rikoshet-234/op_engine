#include "pch_script.h"
#include "UIEditBox.h"
#include "UIEditBoxEx.h"
#include "../ai_space.h"
#include "../script_storage_space.h"
#include "../script_engine.h"

using namespace luabind;

#pragma optimize("s",on)
void CUIEditBox_SetText(CUIEditBox* self_box,LPCSTR text)
{
	self_box->SetText(text);
}

void CUIEditBox_SetText_number(CUIEditBox* self_box,float number)
{
	ai().script_engine().script_log		(ScriptStorage::eLuaMessageTypeError,"SetText : invalid input type! use lua tostring() function for save value precision!!!");
}

void CUIEditBox_SetText_bool(CUIEditBox* self_box,bool value)
{
	self_box->SetText(value ? "true" : "false");
}

void CUIEditBox::script_register(lua_State *L)
{
	module(L)
	[
		class_<CUICustomEdit, CUIWindow>("CUICustomEdit")
		.def("SetText",				&CUICustomEdit::SetText)
		.def("GetText",				&CUICustomEdit::GetText)
		.def("SetTextColor",		&CUICustomEdit::SetTextColor)
		.def("GetTextColor",		&CUICustomEdit::GetTextColor)
		.def("SetFont",				&CUICustomEdit::SetFont)
		.def("GetFont",				&CUICustomEdit::GetFont)
		.def("SetTextAlignment",	&CUICustomEdit::SetTextAlignment)
		.def("GetTextAlignment",	&CUICustomEdit::GetTextAlignment)
		.def("SetTextPosX",			&CUICustomEdit::SetTextPosX)
		.def("SetTextPosY",			&CUICustomEdit::SetTextPosY)
		.def("SetNumbersOnly",		&CUICustomEdit::SetNumbersOnly),

		class_<CUIEditBox, CUICustomEdit>("CUIEditBox")
		.def(						constructor<>())
		.def("SetText",				&CUIEditBox_SetText)
		.def("SetText",				&CUIEditBox_SetText_number)
		.def("SetText",				&CUIEditBox_SetText_bool)
		.def("InitTexture",			&CUIEditBox::InitTexture),

		class_<CUIEditBoxEx, CUICustomEdit>("CUIEditBoxEx")
		.def(						constructor<>())
		.def("InitTexture",			&CUIEditBoxEx::InitTexture)
	];
}