#ifndef UIExoBatterStatic_h
#define UIExoBatterStatic_h

#pragma once
#include "UIStatic.h"

class CUIOutfitDragDropList;

class CUIExoBatteryStatic: public CUIStatic
{
private:
	typedef CUIStatic inherited;
	CUIOutfitDragDropList* parentItem;
public:
	CUIExoBatteryStatic();
	bool OnDbClick() override;
	bool OnMouse(float x, float y, EUIMessages mouse_action) override;

	void SetParentItem(CUIOutfitDragDropList* cell) { parentItem = cell; }
};
#endif
