#include "pch_script.h"
#include "UIListItemIconed.h"

using namespace luabind;

void SetIconedRowData(CUIListItemIconed* row,LPCSTR index)
{
	row->SetData((void*)index);
}

void CUIListItemIconed::script_register(lua_State *L)
{
	module(L)
	[
		class_<CUIListItemIconed>("CUIListItemIconed")
		.def(						constructor<>())
		.def("SetData",			&SetIconedRowData)
		.def("SetFieldText",	&CUIListItemIconed::SetFieldText)
		.def("GetFieldText",	&CUIListItemIconed::GetFieldText)
		.def("SetFieldIcon",	&CUIListItemIconed::SetFieldIcon)
		.def("ClearField",		&CUIListItemIconed::ClearField)
		.def("SetVisibility",	&CUIListItemIconed::SetVisibility)
		.def("GetVisibility",	&CUIListItemIconed::GetVisibility)
	];
};
