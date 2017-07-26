#pragma once
#include "UICellItem.h"
#include "../Weapon.h"


class CUISectionCellItem:public CUICellItem
{
private:
	typedef  CUICellItem	inherited;
public:
	CUISectionCellItem(shared_str section);
	bool			EqualTo(CUICellItem* itm) override;
	void			Update() override;
	CInventoryItem* object() const { return nullptr; }
};

class CUIInventoryCellItem :public CUICellItem
{
	typedef  CUICellItem	inherited;
public:
								CUIInventoryCellItem		(CInventoryItem* itm);
	virtual		bool			EqualTo						(CUICellItem* itm);
				CInventoryItem* object						() {return (CInventoryItem*)m_pData;}
};

class CUIAmmoCellItem :public CUIInventoryCellItem
{
	typedef  CUIInventoryCellItem	inherited;
protected:
	virtual		void			UpdateItemText			();
public:
								CUIAmmoCellItem				(CWeaponAmmo* itm);
	virtual		void			Update						();
	virtual		bool			EqualTo						(CUICellItem* itm);
				CWeaponAmmo*	object						() {return (CWeaponAmmo*)m_pData;}
};


class CUIWeaponCellItem :public CUIInventoryCellItem
{
	typedef  CUIInventoryCellItem	inherited;
public:
	enum eAddonType{	eSilencer=0, eScope, eLauncher, eMaxAddon};
protected:
	CUIStatic*					m_addons					[eMaxAddon];
	Fvector2					m_addon_offset				[eMaxAddon];
	void						CreateIcon					(eAddonType);
	void						DestroyIcon					(eAddonType);
	CUIStatic*					GetIcon						(eAddonType);
	void						InitAddon					(CUIStatic* s, LPCSTR section, Fvector2 offset);
	bool						is_scope					();
	bool						is_silencer					();
	bool						is_launcher					();
public:
								CUIWeaponCellItem			(CWeapon* itm);
				virtual			~CUIWeaponCellItem			();
	virtual		void			Update						();
				CWeapon*		object						() {return (CWeapon*)m_pData;}
	virtual		void			OnAfterChild				();
	virtual		CUIDragItem*	CreateDragItem				();
	virtual		bool			EqualTo						(CUICellItem* itm);
	CUIStatic*					get_addon_static			(u32 idx)				{return m_addons[idx];}
	Fvector2					get_addon_offset			(u32 idx)				{ return m_addon_offset[idx]; }
};

class CBuyItemCustomDrawCell :public ICustomDrawCell
{
	CGameFont*			m_pFont;
	string16			m_string;
public:
						CBuyItemCustomDrawCell	(LPCSTR str, CGameFont* pFont);
	virtual void		OnDraw					(CUICellItem* cell);

};
