#include "stdafx.h"
#include "UICellItemFactory.h"
#include "UICellCustomItems.h"
#include "../exooutfit.h"


CUICellItem*	create_cell_item(CInventoryItem* itm)
{

	CWeaponAmmo* pAmmo = smart_cast<CWeaponAmmo*>(itm);
	if (pAmmo)							
		return xr_new<CUIAmmoCellItem>(pAmmo);

	CWeapon* pWeapon = smart_cast<CWeapon*>(itm);
	if (pWeapon)							
		return xr_new<CUIWeaponCellItem>(pWeapon);

	CExoOutfit* exo = smart_cast<CExoOutfit*>(itm);
	if (exo)
		return xr_new<CExoOutfitCellItem>(exo);

	return xr_new<CUIInventoryCellItem>(itm);
}

CUICellItem* create_cell_item(shared_str itemSection)
{
	return xr_new<CUISectionCellItem>(itemSection);
}


