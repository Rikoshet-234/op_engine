#include "pch_script.h"
#include "UIPropertiesBox.h"
#include "UIListBoxItem.h"


#pragma optimize("s",on)
bool AddItem_script(CUIPropertiesBox *pself,const char*  str)
{
	return pself->AddItem(str,nullptr,900);
};


bool InsertItem_script(CUIPropertiesBox *pself, int pos,const char*  str)
{

		CUIListBoxItem* new_item = pself->m_UIListWnd.AddItem(str);
		new_item->SetTAG(900);
		new_item->SetData(nullptr);


		ui_list<CUIWindow*> *windows = pself->m_UIListWnd.GetItemsList();
		ui_list<CUIWindow*>::reverse_iterator it = windows->rbegin();
		ui_list<CUIWindow*>::reverse_iterator it_e = windows->rend();
		ui_list<CUIWindow*>::reverse_iterator it_prev = it;
		clamp(pos, 0, static_cast<int>(windows->size()));
		for (; it != it_e; ++it)
		{
			if (*it == new_item)
			{
				it_prev = it;
				++it_prev;
				if (it_prev == it_e)	break;
				std::swap(*it, *it_prev);
				int curr_pos = std::distance(windows->rbegin().base(), it_prev.base());
				if (curr_pos-1 == static_cast<int>(pos))
					break;
			}

		}
		pself->m_UIListWnd.ForceUpdate();
		return true;
};


void CUIPropertiesBox::script_register(lua_State *L)
{
	luabind::module(L)
	[
		luabind::class_<CUIPropertiesBox,CUIFrameWindow>("CUIPropertiesBox")
		.def(luabind::constructor<>())
		.def("RemoveItem",			&CUIPropertiesBox::RemoveItemByTAG)
		.def("GetItemsCount",		&CUIPropertiesBox::GetItemsCount)
		.def("RemoveAll",			&CUIPropertiesBox::RemoveAll)
		.def("Show",				(void(CUIPropertiesBox::*)(int,int))&CUIPropertiesBox::Show)
		.def("Hide",				&CUIPropertiesBox::Hide)
//		.def("GetClickedIndex",		&CUIPropertiesBox::GetClickedIndex)
		.def("AutoUpdateSize",		&CUIPropertiesBox::AutoUpdateSize)

		.def("AddItem",				&AddItem_script)
		.def("InsertItem", &InsertItem_script)
	];
}
