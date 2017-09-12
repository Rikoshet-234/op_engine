#pragma once

#include "UIStatic.h"
#include "UIDialogWnd.h"
#include "../inventory_item.h"
#include "UIColorAnimatorWrapper.h"
#include "UIProgressBar.h"

class CUIDragItem;
class CUIDragDropListEx;
class CUICellItem;

class ICustomDrawCell
{
public:
	virtual				~ICustomDrawCell	()	{};
	virtual void		OnDraw				(CUICellItem* cell)	= 0;
};

class CUICellItem :public CUIStatic
{
private:
	typedef		CUIStatic	inherited;
protected:
	xr_vector<CUICellItem*> m_childs;

	bool					m_moveableToOther; //Enable(false) not receive any input from mouse 
	bool					m_allowedGrouping;
	CUIDragDropListEx*		m_pParentList;
	Ivector2				m_grid_size;
	Ivector2				m_grid_size_start;
	ICustomDrawCell*		m_custom_draw;
	int						m_accelerator;
	CUIProgressBar*			p_ConditionProgressBar;
	virtual void			UpdateItemText			();

public:
							CUICellItem				();
	virtual					~CUICellItem			();

	virtual		bool		OnKeyboard				(int dik, EUIMessages keyboard_action);
	virtual		bool		OnMouse					(float x, float y, EUIMessages mouse_action);
	virtual		void		Draw					();
	virtual		void		Update					()						{inherited::Update(); m_b_already_drawn=false;};
				
	virtual		void		OnAfterChild			()						{};

				u32			ChildsCount				() const;
				bool		HasChilds() const		{return m_childs.size()>0;}
				void		 PushChild				(CUICellItem*);
				CUICellItem* PopChild				();
				CUICellItem* Child					(u32 idx)				{return m_childs[idx];};
				bool		HasChild					(CUICellItem* item);
	virtual		bool		EqualTo					(CUICellItem* itm);
	IC const	Ivector2&	GetGridSize				() const {return m_grid_size;}; //size in grid
	IC			void		SetGridSize				(Ivector2 vec)			{m_grid_size.set(vec);}
	IC			void		SetGridWidth			(int width)				{m_grid_size.x=width;}
	IC			void		SetGridHeight			(int height)			{m_grid_size.y=height;}
	IC			void		ResetGridSize			()						{m_grid_size.set(m_grid_size_start);}
	IC			void		SetAccelerator			(int dik)				{m_accelerator=dik;};
	IC			int			GetAccelerator			()		const			{return m_accelerator;};
	
	virtual		CUIDragItem* CreateDragItem			();
				bool		GetAllowedGrouping	() const		{return m_allowedGrouping;}
				void		SetAllowedGrouping	(bool value)	{m_allowedGrouping=value;}
				bool		GetMoveableToOther	() const		{return m_moveableToOther;}
				void		SetMoveableToOther	(bool value)	{m_moveableToOther=value;}

	CUIDragDropListEx*		OwnerList				()						{return m_pParentList;}
				void		SetOwnerList			(CUIDragDropListEx* p);
				void		SetCustomDraw			(ICustomDrawCell* c);
				size_t			m_drawn_frame;
				void*		m_pData;
	CInventoryItem*			GetInventoryItem() const {return	static_cast<PIItem>(m_pData);}
				int			m_index;
				bool		m_b_already_drawn;
	bool					m_b_destroy_childs;
	bool					m_focused;
	bool					m_selected;
	bool					m_suitable;

	bool m_bIgnoreItemPlace;
	Fcolor					m_preAnimTexColor;
	Fcolor					m_preAnimTextColor;
	void					SaveColors();
	void					RestoreColors();
	void SetFocused(bool value) { m_focused = value; }
	void SetSelected(bool value);

	LPCSTR GetCellSection();
	UIIconInfo m_iconInfo;
	shared_str m_sSection;
};

class CUIDragItem: public CUIWindow, public pureRender, public pureFrame
{
private:
	typedef		CUIWindow	inherited;
	CUIStatic				m_static;
	CUICellItem*			m_pParent;
	Fvector2				m_pos_offset;
	CUIDragDropListEx*		m_back_list;
public:
							CUIDragItem(CUICellItem* parent);
	virtual		void		Init(const ref_shader& sh, const Frect& rect, const Frect& text_rect);
	virtual					~CUIDragItem();
			CUIStatic*		wnd						() {return &m_static;}
	virtual		bool		OnMouse					(float x, float y, EUIMessages mouse_action);
	virtual		void		Draw					();
	virtual		void		OnRender				();
	virtual		void		OnFrame					();
		CUICellItem*		ParentItem				()							{return m_pParent;}
				void		SetBackList				(CUIDragDropListEx*l);
	CUIDragDropListEx*		BackList				()							{return m_back_list;}
				Fvector2	GetPosition				();
};
