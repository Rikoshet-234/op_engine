#pragma once

#include "UIWindow.h"
#include "UIWndCallback.h"
#include "../inventory_space.h"
#include "UIProgressBar.h"


class CInventoryItem;
class CUICellContainer;
class CUIScrollBar;
class CUIStatic;
class CUICellItem;
class CUIDragItem;

enum EListType{
		iwSlot,
		iwBag,
		iwBelt
};

struct CUICell{
							CUICell					()						{m_item=nullptr; Clear();}

		CUICellItem*		m_item;
		bool				m_bMainItem;
		void				SetItem					(CUICellItem* itm, bool bMain)		{m_item = itm; VERIFY(m_item);m_bMainItem = bMain;}
		bool				Empty					()						{return m_item == nullptr;}
		bool				MainItem				()						{return m_bMainItem;}
		void				Clear					();
		bool				operator ==				(const CUICell& C) const{return (m_item == C.m_item);}
};

typedef xr_vector<CUICell>			UI_CELLS_VEC;
typedef UI_CELLS_VEC::iterator		UI_CELLS_VEC_IT;

typedef enum //inventory list types for export to scripts (identically to InventorySlots, but plus more items
{
	ltSlotKnife			= KNIFE_SLOT,
	ltSlotPistol		= PISTOL_SLOT,
	ltSlotRifle			= RIFLE_SLOT,
	ltGrenade			= GRENADE_SLOT,
	ltSlotApparatus		= APPARATUS_SLOT,
	ltBolt				= BOLT_SLOT,
	ltSlotOutfit		= OUTFIT_SLOT,
	ltPDA				= PDA_SLOT,
	ltSlotDetectorArts	= DETECTOR_ARTS_SLOT,
	ltTorch				= TORCH_SLOT,
	ltSlotDetectorAnoms = DETECTOR_ANOM_SLOT,
	ltSlotPNV			= PNV_SLOT,
	ltSlotShotgun		= SHOTGUN_SLOT,
	ltSlotBiodev		= BIODEV_SLOT,
	ltBag				= 20,
	ltBelt				= 21,
	ltTradeOurBag		= 22,
	ltTradeOtherBag		= 23,
	ltTradeOurTrade		= 24,
	ltTradeOtherTrade	= 25,
	ltCarbodyOurBag		= 26,
	ltCarbodyOtherBag		= 27,
	ltUnknown			= -1
} IWListTypes;

enum TPosition
{
	left,
	right,
	top,
	bottom,
	left_top,
	left_bottom,
	right_top,
	right_bottom
};

struct TCachedData
{
	bool initialized;
	TPosition position;
	float fatness;
	float min_pos;
	float max_pos;
	float pos;
	float inertion;
	float indent;
	u32 min_color;
	u32 max_color;
	shared_str texture;
	bool stretchTexture;
	shared_str textureBack;
	bool stretchTextureBack;
	
} ;

class CUIDragDropListEx :public CUIWindow, public CUIWndCallback
{
private:
	typedef CUIWindow inherited;


	enum{	
		flGroupSimilar		=	(1<<0),
		flAutoGrow			=	(1<<1),
		flCustomPlacement	=	(1<<2)
	};
	Flags8					m_flags;
	CUICellItem*			m_selected_item;
	Ivector2				m_orig_cell_capacity;
	bool					m_b_showConditionBar;;

protected:
	
	CUICellContainer*		m_container;
	CUIScrollBar*			m_vScrollBar;

	void	__stdcall		OnScrollV				(CUIWindow* w, void* pData);
	void	__stdcall		OnItemStartDragging		(CUIWindow* w, void* pData);
	void	__stdcall		OnItemDrop				(CUIWindow* w, void* pData);
	void	__stdcall		OnItemSelected			(CUIWindow* w, void* pData);
	void	__stdcall		OnItemRButtonClick		(CUIWindow* w, void* pData);
	void	__stdcall		OnItemDBClick			(CUIWindow* w, void* pData);
	void	__stdcall		OnItemFocusReceived		(CUIWindow* w, void* pData);
	void	__stdcall		OnItemFocusLost			(CUIWindow* w, void* pData);

	IWListTypes				listId;
public:
	TCachedData cacheData;
	bool					GetShowConditionBar() const {return m_b_showConditionBar;}
	void SetShowConditionBar(bool state);
	
	IWListTypes				GetUIListId() const			{return listId; };
	void					SetUIListId(IWListTypes id)	{listId=id; };
	bool					m_b_adjustCells;
	static CUIDragItem*		m_drag_item;
	int						m_i_scroll_pos;
							CUIDragDropListEx	();
	virtual					~CUIDragDropListEx	();
	void					Init				(float x, float y, float w, float h);

	typedef					fastdelegate::FastDelegate1<CUICellItem*, bool>		DRAG_DROP_EVENT;

	DRAG_DROP_EVENT			m_f_item_drop;
	DRAG_DROP_EVENT			m_f_item_start_drag;
	DRAG_DROP_EVENT			m_f_item_db_click;
	DRAG_DROP_EVENT			m_f_item_selected;
	DRAG_DROP_EVENT			m_f_item_rbutton_click;
	DRAG_DROP_EVENT			m_f_item_focus_received;
	DRAG_DROP_EVENT			m_f_item_focus_lost;

	bool			select_suitables_by_selected();
	bool			select_suitables_by_item(CInventoryItem* item);
	bool			select_weapons_by_addon(CInventoryItem* addonItem);
	bool			select_weapons_by_ammo(CInventoryItem* ammoItem);
	bool			select_items_by_section(shared_str section);


	const	Ivector2&		CellsCapacity		();
			void			SetCellsCapacity	(const Ivector2 c);
			void			SetStartCellsCapacity(const Ivector2 c){m_orig_cell_capacity=c;SetCellsCapacity(c);};
			void			ResetCellsCapacity	(){VERIFY(ItemsCount()==0);SetCellsCapacity(m_orig_cell_capacity);};
	 const	Ivector2&		CellSize			();
			void			SetCellSize			(const Ivector2 new_sz);
			int				ScrollPos			();
			void			SetScrollPos		(int iPos);
			void			ReinitScroll		();
			void			GetClientArea		(Frect& r);
			Fvector2		GetDragItemPosition	();

			void			SetAutoGrow			(bool b);
			bool			IsAutoGrow			();
			void			SetGrouping			(bool b);
			bool			IsGrouping			();
			void			SetCustomPlacement	(bool b);
			bool			GetCustomPlacement	();
public:
			// items management
			virtual void	SetItem				(CUICellItem* itm); //auto
			virtual void	SetItem				(CUICellItem* itm, Fvector2 abs_pos);  // start at cursor pos
			virtual void	SetItem				(CUICellItem* itm, Ivector2 cell_pos); // start at cell
					bool	CanSetItem			(CUICellItem* itm);

			u32				ItemsCount			();
			CUICellItem*	GetItemIdx			(u32 idx);
	virtual CUICellItem*	RemoveItem			(CUICellItem* itm, bool force_root);
			void			CreateDragItem		(CUICellItem* itm);

			void			DestroyDragItem		();
			void			ClearAll			(bool bDestroy);	
			void			Compact				();
			bool			IsOwner				(CUICellItem* itm);
public:
	//UIWindow overriding
	virtual		void		Draw				();
	virtual		void		Update				();
	virtual		bool		OnMouse				(float x, float y, EUIMessages mouse_action);
	virtual		void		SendMessage			(CUIWindow* pWnd, s16 msg, void* pData = nullptr);
	
	CUICellContainer*	GetCellContainer() const {return m_container;}
};

class CUICellContainer :public CUIWindow
{
	friend class CUIDragDropListEx;

private:
	typedef CUIWindow inherited;
	ref_shader					hShader;  //ownerDraw
	ref_geom					hGeom;	
	UI_CELLS_VEC				m_cells_to_draw;
protected:
	CUIDragDropListEx*			m_pParentDragDropList;

	Ivector2					m_cellsCapacity;			//count		(col,	row)
	Ivector2					m_cellSize;					//pixels	(width, height)
	UI_CELLS_VEC				m_cells;
	
	void						GetTexUVLT			(Fvector2& uv, u32 col, u32 row,u8 select_mode=0);
	void						ReinitSize			();
	u32							GetCellsInRange		(const Irect& rect, UI_CELLS_VEC& res);

public:							
								CUICellContainer	(CUIDragDropListEx* parent);
	virtual						~CUICellContainer	();
	CUICellItem*				GetFocusedCellItem		();
	void						clear_select_suitables();

	Fcolor				m_focused_color;
	Fcolor				m_selected_color;
	Fcolor				m_suitable_color;
	s32					m_anim_mSec;

protected:
	virtual		void			Draw				();

	IC const	Ivector2&		CellsCapacity		()								{return m_cellsCapacity;};	
				void			SetCellsCapacity	(const Ivector2& c);
	IC const	Ivector2&		CellSize			()								{return m_cellSize;};	
				void			SetCellSize			(const Ivector2& new_sz);
				Ivector2		TopVisibleCell		();
				CUICell&		GetCellAt			(const Ivector2& pos);
				Ivector2		PickCell			(const Fvector2& abs_pos);
				Ivector2		GetItemPos			(CUICellItem* itm);
				Ivector2		FindFreeCell		(const Ivector2& size);
				bool			HasFreeSpace		(const Ivector2& size);
				bool			IsRoomFree			(const Ivector2& pos, const Ivector2& size);
				
				bool			AddSimilar			(CUICellItem* itm);
				CUICellItem*	FindSimilar			(CUICellItem* itm);
				void			PlaceItemAtPos		(CUICellItem* itm, Ivector2& cell_pos);
				CUICellItem*	RemoveItem			(CUICellItem* itm, bool force_root);
				bool			ValidCell			(const Ivector2& pos) const;

				void			Grow				();
				void			Shrink				();
				void			ClearAll			(bool bDestroy);
};
