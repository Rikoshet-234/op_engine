#include "pch_script.h"
#include "UIPropertiesBox.h"


#pragma optimize("s",on)
void CUIPropertiesBox::script_register(lua_State *L)
{
	luabind::module(L)
	[
		luabind::class_<CUIPropertiesBox,CUIFrameWindow>("CUIPropertiesBox")
		.def(luabind::constructor<>())
//		.def("AddItem",					&CUIPropertiesBox::AddItem)
		.def("RemoveItem",			&CUIPropertiesBox::RemoveItemByTAG)
		.def("GetItemsCount",		&CUIPropertiesBox::GetItemsCount)
		.def("RemoveAll",			&CUIPropertiesBox::RemoveAll)
		.def("Show",				(void(CUIPropertiesBox::*)(int,int))&CUIPropertiesBox::Show)
		.def("Hide",				&CUIPropertiesBox::Hide)
//		.def("GetClickedIndex",		&CUIPropertiesBox::GetClickedIndex)
		.def("AutoUpdateSize",		&CUIPropertiesBox::AutoUpdateSize)
		.def("AddItem",				&CUIPropertiesBox::AddItem_script)
		// ReSharper disable once CppCStyleCast
		//.def("AddSubItem",			(void(CUIPropertiesBox::*)(LPCSTR,luabind::functor<void>&))&CUIPropertiesBox::AddSubItem)

//		.def("",					&CUIPropertiesBox::)
	];
}
