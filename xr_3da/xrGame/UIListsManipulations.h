#ifndef UIListManipulationsH
#define UIListManipulationsH

#include <luabind/luabind.hpp>
#include "ui/UIDragDropListEx.h"


typedef xr_vector<CUIDragDropListEx* > DragDropListVector;

class CUIListManipulations
{
protected:
	DragDropListVector sourceDragDropLists;
	
public:
	CUIListManipulations();
	virtual ~CUIListManipulations() {}

	void ClearSuitablesInList(IWListTypes listType);
	void SetSuitableBySection(LPCSTR section);
	void SetSuitableBySection(luabind::object const& sections);
	void ClearAllSuitables();
	void SetSuitableBySectionInList(IWListTypes listType, luabind::object const& sections);
	void SetSuitableBySectionInList(IWListTypes listType, LPCSTR section);
};

#endif