#include "stdafx.h"
#include "UIExoBatteryStatic.h"
#include "UIOutfitSlot.h"
#include "UICellItem.h"
#include "../exooutfit.h"

CUIExoBatteryStatic::CUIExoBatteryStatic()
{
	parentItem = nullptr;
}

bool CUIExoBatteryStatic::OnDbClick()
{
	if (parentItem)
	{
		CUICellItem* cell = parentItem->GetItemIdx(0);
		if (cell && cell->m_pData)
		{
			PIItem _iitem = static_cast<PIItem>(cell->m_pData);
			//CCustomOutfit* pOutfit = smart_cast<CCustomOutfit*>(_iitem); 
			CExoOutfit* exo = smart_cast<CExoOutfit*>(_iitem);
			if (exo && exo->isBatteryPresent())
				exo->RemoveFromBatterySlot();
		}
		return true;
	}
	return inherited::OnDbClick();
}

bool CUIExoBatteryStatic::OnMouse(float x, float y, EUIMessages mouse_action)
{
	if (mouse_action == WINDOW_LBUTTON_DOWN && parentItem)
	{
		parentItem->SendMessage(parentItem->GetItemIdx(0), DRAG_DROP_ITEM_SELECTED, nullptr);
		return true;
	}
	return inherited::OnMouse(x, y, mouse_action);
}
