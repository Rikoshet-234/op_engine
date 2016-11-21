#include "pch_script.h"

#include "UIListsManipulations.h"
#include "OPFuncs/utils.h"

CUIListManipulations::CUIListManipulations()
{
	
}

void CUIListManipulations::ClearSuitablesInList(IWListTypes listType)
{
	auto listIt=std::find_if(sourceDragDropLists.begin(),sourceDragDropLists.end(),[listType](CUIDragDropListEx* list)
	{
		return list->GetUIListId()==listType;
	});
	if (listIt!=sourceDragDropLists.end())
	{
		(*listIt)->GetCellContainer()->clear_select_suitables();
	}
}

void CUIListManipulations::SetSuitableBySection(LPCSTR section)
{
	std::for_each(sourceDragDropLists.begin(),sourceDragDropLists.end(),[section](CUIDragDropListEx* list)
	{
		list->select_items_by_section(section);
	});
}

void CUIListManipulations::SetSuitableBySection(luabind::object const& sections)
{
	xr_vector<LPCSTR> sectionsList= OPFuncs::getStringsFromLua(sections);
	if (sectionsList.size()>0)
		std::for_each(sectionsList.begin(),sectionsList.end(),[&](LPCSTR section)
		{
			SetSuitableBySection(section);
		});
}

void CUIListManipulations::ClearAllSuitables()
{
	std::for_each(sourceDragDropLists.begin(),sourceDragDropLists.end(),[](CUIDragDropListEx* list)
	{
		list->GetCellContainer()->clear_select_suitables();
	});
}

void CUIListManipulations::SetSuitableBySectionInList(IWListTypes listType, luabind::object const& sections)
{
	auto listIt=std::find_if(sourceDragDropLists.begin(),sourceDragDropLists.end(),[listType](CUIDragDropListEx* list)
	{
		return list->GetUIListId()==listType;
	});
	if (listIt!=sourceDragDropLists.end())
	{
		xr_vector<LPCSTR> listStrings=OPFuncs::getStringsFromLua(sections);
		if (listStrings.size()>0)
			std::for_each(listStrings.begin(),listStrings.end(),[&](LPCSTR section)
			{
				(*listIt)->select_items_by_section(section);
			});
	};
}

void CUIListManipulations::SetSuitableBySectionInList(IWListTypes listType, LPCSTR section)
{
	auto listIt=std::find_if(sourceDragDropLists.begin(),sourceDragDropLists.end(),[listType](CUIDragDropListEx* list)
	{
		return list->GetUIListId()==listType;
	});
	if (listIt!=sourceDragDropLists.end())
	{
		(*listIt)->select_items_by_section(section);
	};
}
