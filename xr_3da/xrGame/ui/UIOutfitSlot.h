#pragma once
#include "UIDragDropListEx.h"
#include "UIExoBatteryStatic.h"

class CUIOutfitDragDropList :public CUIDragDropListEx
{
	typedef CUIDragDropListEx						inherited;
	CUIStatic*										m_background;
	bool m_bBackgroundInitialized;

	CUIStatic* m_pBatteryText;
	CUIExoBatteryStatic* m_pBatteryIcon;
	CUIFrameWindow*	m_pBatteryIconBackground;
	CUIProgressBar* m_pChargeBatteryProgress;
	shared_str										m_default_outfit;
	bool m_bDrawBatteryPart;
	void					SetOutfit				(CUICellItem* itm);
	
public:
							CUIOutfitDragDropList	();
	virtual					~CUIOutfitDragDropList	();

	virtual void			SetItem					(CUICellItem* itm); //auto
	virtual void			SetItem					(CUICellItem* itm, Fvector2 abs_pos);  // start at cursor pos
	virtual void			SetItem					(CUICellItem* itm, Ivector2 cell_pos); // start at cell
	virtual CUICellItem*	RemoveItem				(CUICellItem* itm, bool force_root);
	virtual	void			Draw					();
			void			SetDefaultOutfit		(LPCSTR default_outfit);

			void UIInitBattery(CUIXml& xml_doc, const char* rootPath);
};
