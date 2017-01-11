#include "stdafx.h"
#include "UIDragDropListEx.h"
#include "UIScrollBar.h"
#include "../object_broker.h"
#include "UICellItem.h"
#include "../../defines.h"
#include "../inventory_item.h"

#include "../Weapon.h"
#include "../WeaponMagazinedWGrenade.h"
#include "../WeaponAmmo.h"
#include "../Silencer.h"
#include "../Scope.h"
#include "../GrenadeLauncher.h"
#include "../CustomOutfit.h"
#include "../eatable_item.h"
#include "../actor.h"
#include "../game_object_space.h"
#include "../script_callback_ex.h"
#include "../script_game_object.h"
#include "xrUIXmlParser.h"
#include "UIXmlInit.h"


CUIDragItem* CUIDragDropListEx::m_drag_item = NULL;

struct TCachedData
{
	bool initialized;
	float x;
	float y;
	float width;
	float height;
	float min_pos;
	float max_pos;
	float pos;
	float inertion;
	u32 min_color;
	u32 max_color;
	shared_str texture;
	bool stretchTexture;
	shared_str textureBack;
	bool stretchTextureBack;
	
} ;
static TCachedData cacheData;

void CUICell::Clear()
{
	m_bMainItem = false;
	if(m_item)	m_item->SetOwnerList(nullptr);
	m_item		= nullptr; 
}

CUIDragDropListEx::CUIDragDropListEx()
{
	listId=ltUnknown;
	m_flags.zero				();
	m_container					= xr_new<CUICellContainer>(this);
	m_vScrollBar				= xr_new<CUIScrollBar>();
	m_vScrollBar->SetAutoDelete	(true);
	m_selected_item				= nullptr;
	m_b_showConditionBar		= false;

	SetCellSize					(Ivector2().set(50,50));
	SetCellsCapacity			(Ivector2().set(0,0));

	CUIWindow::AttachChild					(m_container);
	CUIWindow::AttachChild					(m_vScrollBar);

	m_vScrollBar->SetWindowName	("scroll_v");
	Register					(m_vScrollBar);
	AddCallback					("scroll_v",	SCROLLBAR_VSCROLL,				CUIWndCallback::void_function				(this, &CUIDragDropListEx::OnScrollV)		);
	AddCallback					("cell_item",	DRAG_DROP_ITEM_DRAG,			CUIWndCallback::void_function			(this, &CUIDragDropListEx::OnItemStartDragging)	);
	AddCallback					("cell_item",	DRAG_DROP_ITEM_DROP,			CUIWndCallback::void_function			(this, &CUIDragDropListEx::OnItemDrop)			);
	AddCallback					("cell_item",	DRAG_DROP_ITEM_SELECTED,		CUIWndCallback::void_function		(this, &CUIDragDropListEx::OnItemSelected)			);
	AddCallback					("cell_item",	DRAG_DROP_ITEM_RBUTTON_CLICK,	CUIWndCallback::void_function	(this, &CUIDragDropListEx::OnItemRButtonClick)			);
	AddCallback					("cell_item",	DRAG_DROP_ITEM_DB_CLICK,		CUIWndCallback::void_function		(this, &CUIDragDropListEx::OnItemDBClick)			);
	AddCallback					("cell_item",	WINDOW_FOCUS_RECEIVED,			CUIWndCallback::void_function		(this, &CUIDragDropListEx::OnItemFocusReceived)			);
	AddCallback					("cell_item",	WINDOW_FOCUS_LOST,				CUIWndCallback::void_function		(this, &CUIDragDropListEx::OnItemFocusLost)			);
	m_i_scroll_pos=-1;
	cacheData.initialized=false;
}

CUIDragDropListEx::~CUIDragDropListEx()
{
	DestroyDragItem		();

	delete_data					(m_container);
}

void CUIDragDropListEx::SetAutoGrow(bool b)						
{
	m_flags.set(flAutoGrow,b);
}
bool CUIDragDropListEx::IsAutoGrow()								
{
	return !!m_flags.test(flAutoGrow);
}
void CUIDragDropListEx::SetGrouping(bool b)						
{
	m_flags.set(flGroupSimilar,b);
}
bool CUIDragDropListEx::IsGrouping()
{
	return !!m_flags.test(flGroupSimilar);
}
void CUIDragDropListEx::SetCustomPlacement(bool b)
{
	m_flags.set(flCustomPlacement,b);
}

bool CUIDragDropListEx::GetCustomPlacement()
{
	return !!m_flags.test(flCustomPlacement);
}

void CUIDragDropListEx::SendMessage(CUIWindow* pWnd, s16 msg, void* pData)
{
	CUIWndCallback::OnEvent(pWnd, msg, pData);
}

void CUIDragDropListEx::Init(float x, float y, float w, float h)
{
	inherited::SetWndRect				(x,y,w,h);
	m_vScrollBar->Init					(w, 0, h, false);
	m_vScrollBar->SetWndPos				(m_vScrollBar->GetWndPos().x - m_vScrollBar->GetWidth(), m_vScrollBar->GetWndPos().y);
/*
//.	m_background->Init					(0,0,w,h);
//.	m_background->Init					("ui\\ui_frame_02_back",0,0,w,h);
//.	Frect rect; rect.set				(0,0,64,64);
//.	m_background->GetUIStaticItem().SetOriginalRect(rect);
//.	m_background->SetStretchTexture		(true);
*/
}

void CUIDragDropListEx::OnScrollV(CUIWindow* w, void* pData)
{
	m_container->SetWndPos		(m_container->GetWndPos().x, float(-m_vScrollBar->GetScrollPos()));
}

void CUIDragDropListEx::CreateDragItem(CUICellItem* itm)
{
	R_ASSERT							(!m_drag_item);
	m_drag_item							= itm->CreateDragItem();
	if ( m_drag_item )
		GetParent()->SetCapture				(m_drag_item, true);
}

void CUIDragDropListEx::DestroyDragItem()
{
	if(m_selected_item && m_drag_item && m_drag_item->ParentItem()==m_selected_item)
	{
		VERIFY(GetParent()->GetMouseCapturer()==m_drag_item);
		GetParent()->SetCapture				(NULL, false);

		delete_data							(m_drag_item);
	}
}

Fvector2 CUIDragDropListEx::GetDragItemPosition()
{
	return m_drag_item->GetPosition();
}

void CUIDragDropListEx::OnItemStartDragging(CUIWindow* w, void* pData)
{
	OnItemSelected						(w, pData);
	CUICellItem* itm		= smart_cast<CUICellItem*>(w);

	if(itm!=m_selected_item)	return;
	
	if(m_f_item_start_drag && m_f_item_start_drag(itm) ) return;

	CreateDragItem						(itm);
}

void CUIDragDropListEx::OnItemDrop(CUIWindow* w, void* pData)
{ 
	OnItemSelected						(w, pData);
	CUICellItem*		itm				= smart_cast<CUICellItem*>(w);
	VERIFY								(itm->OwnerList() == itm->OwnerList());

	if(m_f_item_drop && m_f_item_drop(itm) ){
		DestroyDragItem						();
		return;
	}

	CUIDragDropListEx*	old_owner		= itm->OwnerList();
	CUIDragDropListEx*	new_owner		= m_drag_item->BackList();

	bool b				= (old_owner==new_owner)&&!GetCustomPlacement();

	if(old_owner&&new_owner && !b)
	{
		CUICellItem* i					= old_owner->RemoveItem(itm, (old_owner==new_owner) );
		while(i->ChildsCount())
		{
			CUICellItem* _chld				= i->PopChild();
			new_owner->SetItem				(_chld, old_owner->GetDragItemPosition());
		}
		new_owner->SetItem				(i,old_owner->GetDragItemPosition());
	}
	DestroyDragItem						();
}

void CUIDragDropListEx::OnItemDBClick(CUIWindow* w, void* pData)
{
	OnItemSelected						(w, pData);
	CUICellItem*		itm				= smart_cast<CUICellItem*>(w);

	if(m_f_item_db_click && m_f_item_db_click(itm) ){
		DestroyDragItem						();
		return;
	}

	CUIDragDropListEx*	old_owner		= itm->OwnerList();
	VERIFY								(m_drag_item==NULL);
	VERIFY								(old_owner == this);

	if(old_owner&&old_owner->GetCustomPlacement())
	{
		CUICellItem* i					= old_owner->RemoveItem(itm, true);
		old_owner->SetItem				(i);
	}

	DestroyDragItem						();
}

void CUIDragDropListEx::OnItemFocusReceived(CUIWindow* w, void* pData)
{
	if(m_f_item_focus_received)
	{
		CUICellItem* itm				= smart_cast<CUICellItem*>(w);
		m_f_item_focus_received			(itm);
	}

}

void CUIDragDropListEx::OnItemFocusLost(CUIWindow* w, void* pData)
{
	if(m_f_item_focus_lost)
	{
		CUICellItem* itm				= smart_cast<CUICellItem*>(w);
		m_f_item_focus_lost				(itm);
	}

}

void CUIDragDropListEx::PutConditionBarUIData(CUIProgressBar* progress)
{
	if (progress)
	{
		progress->Init(cacheData.x,cacheData.y,cacheData.width,cacheData.height,true);
		progress->SetRange(cacheData.min_pos,cacheData.max_pos);
		progress->SetProgressPos(cacheData.pos);

		progress->m_inertion		= cacheData.inertion;

		progress->m_UIProgressItem.SetOriginalRect(cacheData.x,cacheData.y,cacheData.width,cacheData.height);
		progress->m_UIProgressItem.InitTexture(cacheData.texture.c_str());
		progress->m_UIProgressItem.SetWndSize(progress->GetWndSize());
		//progress->m_UIProgressItem.SetStretchTexture(cacheData.stretchTexture);
		
		progress->m_UIBackgroundItem.SetOriginalRect(cacheData.x,cacheData.y,cacheData.width,cacheData.height);
		progress->m_UIBackgroundItem.InitTexture(cacheData.textureBack.c_str());
		progress->m_UIBackgroundItem.SetWndSize(progress->GetWndSize());
		//progress->m_UIBackgroundItem.SetStretchTexture(cacheData.stretchTextureBack);

		progress->m_minColor.set(cacheData.min_color);
		progress->m_maxColor.set(cacheData.max_color);
		progress->m_bUseColor=true;
	}
}

void CUIDragDropListEx::SetShowConditionBar(bool state)
{
	m_b_showConditionBar = state;
	if (m_b_showConditionBar && !cacheData.initialized)
	{
		CUIXml	uiXml;
		uiXml.Init( CONFIG_PATH, UI_PATH, "cell_item.xml" );
		cacheData.x = uiXml.ReadAttribFlt("progress_item_condition", 0, "x");
		cacheData.y = uiXml.ReadAttribFlt("progress_item_condition", 0, "y");
		cacheData.width = uiXml.ReadAttribFlt("progress_item_condition", 0, "width");
		cacheData.height = uiXml.ReadAttribFlt("progress_item_condition", 0, "height");
		cacheData.min_pos = uiXml.ReadAttribFlt("progress_item_condition", 0, "min");
		cacheData.max_pos = uiXml.ReadAttribFlt("progress_item_condition", 0, "max");
		cacheData.pos = uiXml.ReadAttribFlt("progress_item_condition", 0, "pos");
		cacheData.texture=uiXml.Read("progress_item_condition:progress:texture", 0, nullptr);
		cacheData.stretchTexture=uiXml.ReadAttribInt("progress_item_condition:progress:texture", 0, "stretch")==1?true:false;
		cacheData.textureBack=uiXml.Read("progress_item_condition:background:texture", 0, nullptr);
		cacheData.stretchTextureBack=uiXml.ReadAttribInt("progress_item_condition:background:texture", 0, "stretch")==1?true:false;
		cacheData.min_color=CUIXmlInit::GetColor(uiXml,"progress_item_condition:min_color",0,0xff);
		cacheData.max_color=CUIXmlInit::GetColor(uiXml,"progress_item_condition:max_color",0,0xff);
		cacheData.inertion=uiXml.ReadAttribFlt("progress_item_condition", 0, "inertion", 0.0f);
		cacheData.initialized=true;
	}
}

void CUIDragDropListEx::OnItemSelected(CUIWindow* w, void* pData)
{
	m_selected_item						= smart_cast<CUICellItem*>(w);
	VERIFY								(m_selected_item);
	if(m_f_item_selected)
		m_f_item_selected(m_selected_item);
}

void CUIDragDropListEx::OnItemRButtonClick(CUIWindow* w, void* pData)
{
	OnItemSelected						(w, pData);
	CUICellItem*		itm				= smart_cast<CUICellItem*>(w);
	if(m_f_item_rbutton_click) 
		m_f_item_rbutton_click(itm);
}

void CUIDragDropListEx::GetClientArea(Frect& r)
{
	GetAbsoluteRect				(r);
	if(m_vScrollBar->GetVisible())
		r.x2 -= m_vScrollBar->GetWidth	();
}

void CUIDragDropListEx::ClearAll(bool bDestroy)
{
	DestroyDragItem			();
	m_container->ClearAll	(bDestroy);
	m_selected_item			= nullptr;
	m_container->SetWndPos	(0,0);
	ResetCellsCapacity		();
}

void CUIDragDropListEx::Compact()
{
	CUIWindow::WINDOW_LIST	wl		= m_container->GetChildWndList();
	ClearAll						(false);

	CUIWindow::WINDOW_LIST_it it	= wl.begin();
	CUIWindow::WINDOW_LIST_it it_e	= wl.end();
	for(;it!=it_e;++it)
	{
		CUICellItem*	itm			= smart_cast<CUICellItem*>(*it);
		SetItem						(itm);
	}
}


#include "../HUDManager.h"
void CUIDragDropListEx::Draw()
{
	inherited::Draw				();

	if(0 && bDebug)
		{
		CGameFont* F		= UI()->Font()->pFontDI;
		F->SetAligment		(CGameFont::alCenter);
		F->SetHeightI		(0.02f);
		F->OutSetI			(0.f,-0.5f);
		F->SetColor			(0xffffffff);
		Ivector2			pt = m_container->PickCell(GetUICursor()->GetCursorPosition());
		F->OutNext			("%d-%d",pt.x, pt.y);
	};

}

void CUIDragDropListEx::Update()
{
	inherited::Update			();

	if( m_drag_item ){
		Frect	wndRect;
		GetAbsoluteRect(wndRect);
		Fvector2 cp			= GetUICursor()->GetCursorPosition();
		if(wndRect.in(cp)){
			if(NULL==m_drag_item->BackList())
				m_drag_item->SetBackList(this);
		}else
			if( this==m_drag_item->BackList() )
				m_drag_item->SetBackList(NULL);
	}
}

void CUIDragDropListEx::ReinitScroll()
{
		float h1 = m_container->GetWndSize().y;
		float h2 = GetWndSize().y;
		int pos=m_i_scroll_pos==-1 ? ScrollPos() : m_i_scroll_pos;
		VERIFY						(_valid(h1));
		VERIFY						(_valid(h2));
		m_vScrollBar->Show			( h1 > h2 );
		m_vScrollBar->Enable		( h1 > h2 );
		m_vScrollBar->SetRange		(0, _max(0,iFloor(h1-h2)) );
		m_vScrollBar->SetStepSize	(CellSize().y);
		m_vScrollBar->SetPageSize	(iFloor(GetWndSize().y/float(CellSize().y)));

		m_container->SetWndPos		(0,0);
		SetScrollPos	(pos); 
}

bool CUIDragDropListEx::OnMouse(float x, float y, EUIMessages mouse_action)
{
	bool b = inherited::OnMouse		(x,y,mouse_action);

	if(m_vScrollBar->IsShown())
	{
		switch(mouse_action){
		case WINDOW_MOUSE_WHEEL_DOWN:
				m_vScrollBar->TryScrollInc();
				m_i_scroll_pos=m_vScrollBar->GetScrollPos();
				return true;
				break;

		case WINDOW_MOUSE_WHEEL_UP:
				m_vScrollBar->TryScrollDec();
				m_i_scroll_pos=m_vScrollBar->GetScrollPos();
				return true;
				break;
		default: break;
		}
	}
	return b;
}

bool CUIDragDropListEx::select_suitables_by_selected()
{
	if (m_selected_item==nullptr)
		return false;
	CInventoryItem*	item = static_cast<CInventoryItem*>(m_selected_item->m_pData);
	return select_suitables_by_item(item);
}

bool CUIDragDropListEx::select_suitables_by_item(CInventoryItem* item)
{
	if (!item)
		return false;	
	bool selected=false;
	xr_vector<shared_str>	weaponSections;
	CWeapon* pWeapon = smart_cast<CWeapon*>(item);
	if (pWeapon)
	{
		weaponSections.clear_not_free();
		weaponSections.assign( pWeapon->m_ammoTypes.begin(), pWeapon->m_ammoTypes.end() );
		CWeaponMagazinedWGrenade* wg = smart_cast<CWeaponMagazinedWGrenade*>(item);
		if (wg &&  wg->m_ammoTypes2.size() && wg->GrenadeLauncherAttachable())
		{
			weaponSections.insert( weaponSections.end(), wg->m_ammoTypes2.begin(), wg->m_ammoTypes2.end() );
		}
		shared_str scope=pWeapon->GetScopeName();
		shared_str silencer=pWeapon->GetSilencerName();
		shared_str gr_l=pWeapon->GetGrenadeLauncherName();
		if (scope!=nullptr)
			weaponSections.push_back(scope);
		if (silencer!=nullptr)
			weaponSections.push_back(silencer);
		if (gr_l!=nullptr)
			weaponSections.push_back(gr_l);
	}
	selected=select_weapons_by_addon(item) || select_weapons_by_ammo(item);
	if (weaponSections.size()>0)
	{
		std::sort(weaponSections.begin(),weaponSections.end());
		u32 const cnt = this->ItemsCount();
		for ( u32 i = 0; i < cnt; ++i )
		{
			CUICellItem* cellItem = this->GetItemIdx(i);
			CInventoryItem* listItem = static_cast<CInventoryItem*>(cellItem->m_pData);
			if ( !listItem )
				continue;
			xr_vector<shared_str>::iterator fi=weaponSections.begin();
			if (std::find(weaponSections.begin(), weaponSections.end(), listItem->object().cNameSect()) != weaponSections.end())
			{
				cellItem->m_suitable=true;
				selected=true;
			}
		}

	/*	Msg("Need to select [%i] items",itemsSections.size());
		for (auto section = itemsSections.begin(); section != itemsSections.end(); ++section)
		{ 
			Msg("%s",section->c_str());
		}*/
	}
	return selected;
}

bool CUIDragDropListEx::select_weapons_by_addon(CInventoryItem* addonItem)
{
	CScope*				pScope				= smart_cast<CScope*>			(addonItem);
	CSilencer*			pSilencer			= smart_cast<CSilencer*>		(addonItem);
	CGrenadeLauncher*	pGrenadeLauncher	= smart_cast<CGrenadeLauncher*>	(addonItem);
	if (!pScope && !pSilencer && !pGrenadeLauncher)
		return false;
	bool selected=false;
	u32 const cnt = this->ItemsCount();
	for ( u32 i = 0; i < cnt; ++i )
	{
		CUICellItem* ci = this->GetItemIdx(i);
		CInventoryItem* item = static_cast<CInventoryItem*>(ci->m_pData);
		if (!item)
			continue;
		CWeapon* weapon = smart_cast<CWeapon*>(item);
		if (!weapon)
			continue;

		if ( pScope && weapon->ScopeAttachable() && xr_strcmp(weapon->GetScopeName(),pScope->cNameSect())==0)
		{
			ci->m_suitable = true;
			selected=true;
			continue;
		}
		if ( pSilencer && weapon->SilencerAttachable() && xr_strcmp(weapon->GetSilencerName(),pSilencer->cNameSect())==0)
		{
			ci->m_suitable = true;
			selected=true;
			continue;
		}
		if ( pGrenadeLauncher && weapon->GrenadeLauncherAttachable() &&xr_strcmp(weapon->GetGrenadeLauncherName(),pGrenadeLauncher->cNameSect())==0)
		{
			ci->m_suitable = true;
			selected=true;
			continue;
		}
	}
	return selected;
}

bool CUIDragDropListEx::select_weapons_by_ammo(CInventoryItem* ammoItem)
{
	CWeaponAmmo* ammo = smart_cast<CWeaponAmmo*>(ammoItem);
	if (!ammo)
		return false;
	shared_str ammo_name = ammoItem->object().cNameSect();
	bool selected=false;
	u32 const cnt = this->ItemsCount();
	for ( u32 i = 0; i < cnt; ++i )
	{
		CUICellItem* ci = this->GetItemIdx(i);
		CInventoryItem* item = static_cast<CInventoryItem*>(ci->m_pData);
		if (!item)
			continue;
		CWeaponMagazined* weapon = smart_cast<CWeaponMagazined*>(item);
		CWeaponMagazinedWGrenade* wg = smart_cast<CWeaponMagazinedWGrenade*>(item);
		if (wg && wg->CanLoadAmmo(ammo))
		{
			ci->m_suitable = true;
			selected=true;
		}
		else if (weapon && weapon->CanLoadAmmo(ammo))
		{
			ci->m_suitable = true;
			selected=true;
		}
	}
	return selected;
}

bool CUIDragDropListEx::select_items_by_section(shared_str section)
{
	if (!section)
		return false;
	bool selected=false;
	u32 const cnt = this->ItemsCount();
	for ( u32 i = 0; i < cnt; ++i )
	{
		CUICellItem* ci = this->GetItemIdx(i);
		CInventoryItem* item = static_cast<CInventoryItem*>(ci->m_pData);
		if (!item)
			continue;
		if (xr_strcmp(item->object().cNameSect(),section)==0)
		{
			ci->m_suitable = true;
			selected=true;
		}
	}
	return selected;
}

const Ivector2& CUIDragDropListEx::CellsCapacity()
{
	return m_container->CellsCapacity();
}

void CUIDragDropListEx::SetCellsCapacity(const Ivector2 c)
{
	m_container->SetCellsCapacity(c);
}

const Ivector2& CUIDragDropListEx::CellSize()
{
	return m_container->CellSize();
}

void CUIDragDropListEx::SetCellSize(const Ivector2 new_sz)			
{
	m_container->SetCellSize(new_sz);
}

int CUIDragDropListEx::ScrollPos()
{
	return m_vScrollBar->GetScrollPos();
}

void CUIDragDropListEx::SetScrollPos(int iPos)
{
	m_vScrollBar->SetScrollPos(iPos);
	m_vScrollBar->Refresh();
}

void CUIDragDropListEx::SetItem(CUICellItem* itm) //auto
{
	if(m_container->AddSimilar(itm)) return;

	Ivector2 dest_cell_pos =	m_container->FindFreeCell(itm->GetGridSize());
	SetItem						(itm,dest_cell_pos);
}

void CUIDragDropListEx::SetItem(CUICellItem* itm, Fvector2 abs_pos) // start at cursor pos
{
	if(m_container->AddSimilar(itm))	return;

	const Ivector2 dest_cell_pos =	m_container->PickCell		(abs_pos);

	if(m_container->ValidCell(dest_cell_pos) && m_container->IsRoomFree(dest_cell_pos,itm->GetGridSize()))
		SetItem						(itm, dest_cell_pos);
	else
		SetItem						(itm);
}

void CUIDragDropListEx::SetItem(CUICellItem* itm, Ivector2 cell_pos) // start at cell
{
	if(m_container->AddSimilar(itm))	return;
	R_ASSERT						(m_container->IsRoomFree(cell_pos, itm->GetGridSize()));

	if (m_b_adjustCells)
	{
		int itemWidth=itm->GetGridSize().x;
		int itemHeight=itm->GetGridSize().y;
		int contWidth=m_container->CellsCapacity().x;
		int contHeight=m_container->CellsCapacity().y;
		if (itemWidth<contWidth)
			itm->SetGridWidth(contWidth);
		if (itemHeight<contHeight)
			itm->SetGridHeight(contHeight);
	}
	m_container->PlaceItemAtPos	(itm, cell_pos);

	itm->SetWindowName			("cell_item");
	Register					(itm);
	itm->SetOwnerList			(this);
}

bool CUIDragDropListEx::CanSetItem(CUICellItem* itm){
	if (m_container->HasFreeSpace(itm->GetGridSize()))
		return true;
	Compact();

	return m_container->HasFreeSpace(itm->GetGridSize());
}

CUICellItem* CUIDragDropListEx::RemoveItem(CUICellItem* itm, bool force_root)
{
	CUICellItem* i				= m_container->RemoveItem		(itm, force_root);
	i->SetOwnerList				(nullptr);
	i->ResetGridSize();
	return						i;
}

u32 CUIDragDropListEx::ItemsCount()
{
	return m_container->GetChildWndList().size();
}

bool CUIDragDropListEx::IsOwner(CUICellItem* itm){
	return m_container->IsChild(itm);
}

CUICellItem* CUIDragDropListEx::GetItemIdx(u32 idx)
{
	if (idx >= ItemsCount())
	{
		Msg(" attempt get item %d, but items count = %d ", idx, ItemsCount());
		return nullptr;
	}


	WINDOW_LIST_it it = m_container->GetChildWndList().begin();
	std::advance	(it, idx);
	return smart_cast<CUICellItem*>(*it);
}

CUICellContainer::CUICellContainer(CUIDragDropListEx* parent)
{
	m_pParentDragDropList		= parent;
	hShader.create				("hud\\fog_of_war","ui\\ui_grid");
	hGeom.create				(FVF::F_TL, RCache.Vertex.Buffer(), 0);
	m_anim_mSec=0;
	m_suitable_color.set(0xFFFFFFFF);
	m_selected_color.set(0xFFFFFFFF);
	m_focused_color.set(0xFFFFFFFF);
}

CUICellContainer::~CUICellContainer()
{
}

void CUICellContainer::clear_select_suitables()
{
	UI_CELLS_VEC_IT itb = m_cells.begin();
	UI_CELLS_VEC_IT ite = m_cells.end();
	for ( ; itb != ite; ++itb )
	{
		CUICell& cell = (*itb);
		if ( cell.m_item )
		{
			if (cell.m_item->m_suitable)
			{
				if (cell.m_item->IsAnimStarted())
				{
					cell.m_item->SetClrLightAnim(nullptr, false, false, false, true);
					cell.m_item->RestoreColors();
				}
				cell.m_item->m_suitable = false;
			}
		}
	}

}

bool CUICellContainer::AddSimilar(CUICellItem* itm)
{
	if(!m_pParentDragDropList->IsGrouping())	return false;

	CUICellItem* i		= FindSimilar(itm);
	R_ASSERT			(i!=itm);
	R_ASSERT			(0==itm->ChildsCount());
	if(i)
	{	
		i->PushChild			(itm);
		itm->SetOwnerList		(m_pParentDragDropList);
	}
	
	return (i!= nullptr);
}

CUICellItem* CUICellContainer::FindSimilar(CUICellItem* itm)
{
	for(WINDOW_LIST_it it = m_ChildWndList.begin(); m_ChildWndList.end()!=it; ++it)
	{
		CUICellItem* i = static_cast<CUICellItem*>(*it);
		R_ASSERT		(i!=itm);
		if(i->EqualTo(itm) && i->GetAllowedGrouping() && itm->GetAllowedGrouping()) //grouping by single attribute, maybe grouping groups by ID?
			return i;
	}
	return nullptr;
}

CUICellItem* CUICellContainer::GetFocusedCellItem()
{
	for(WINDOW_LIST_it it = m_ChildWndList.begin(); m_ChildWndList.end()!=it; ++it)
	{
		CUICellItem* i = smart_cast<CUICellItem*>(*it);
		if (i->m_focused)
			return i;
	}
	return nullptr;
}

void CUICellContainer::PlaceItemAtPos(CUICellItem* itm, Ivector2& cell_pos)
{
	Ivector2 cs				= itm->GetGridSize();
	for(int x=0; x<cs.x; ++x)
		for(int y=0; y<cs.y; ++y){
			CUICell& C		= GetCellAt(Ivector2().set(x,y).add(cell_pos));
			C.SetItem		(itm,(x==0&&y==0));
		}

	itm->SetWndPos			( Fvector2().set( (m_cellSize.x*cell_pos.x), (m_cellSize.y*cell_pos.y))	);
	itm->SetWndSize			( Fvector2().set( (m_cellSize.x*cs.x),		(m_cellSize.y*cs.y)		 )	);

	AttachChild				(itm);
	itm->OnAfterChild		();
}

CUICellItem* CUICellContainer::RemoveItem(CUICellItem* itm, bool force_root)
{
	for(WINDOW_LIST_it it = m_ChildWndList.begin(); m_ChildWndList.end()!=it; ++it)
	{
		CUICellItem* i		= (CUICellItem*)(*it);
		
		if(i->HasChild(itm))
		{
			CUICellItem* iii	= i->PopChild();
			R_ASSERT			(0==iii->ChildsCount());
			return				iii;
		}
	}

	if(!force_root && itm->ChildsCount())
	{
		CUICellItem* iii	=	itm->PopChild();
		R_ASSERT			(0==iii->ChildsCount());
		return				iii;
	}

	Ivector2 pos			= GetItemPos(itm);
	Ivector2 cs				= itm->GetGridSize();

	for(int x=0; x<cs.x;++x)
		for(int y=0; y<cs.y;++y)
		{
			CUICell& C		= GetCellAt(Ivector2().set(x,y).add(pos));
			C.Clear			();
		}

	itm->SetOwnerList		(nullptr); 
	DetachChild				(itm);
	return					itm;
}

Ivector2 CUICellContainer::FindFreeCell	(const Ivector2& size)
{
	Ivector2 tmp;
	for(tmp.y=0; tmp.y<=m_cellsCapacity.y-size.y; ++tmp.y )
		for(tmp.x=0; tmp.x<=m_cellsCapacity.x-size.x; ++tmp.x )
			if(IsRoomFree(tmp,size))
				return  tmp;

	if(m_pParentDragDropList->IsAutoGrow())
	{
		Grow	();
		return							FindFreeCell	(size);
	}else{
		m_pParentDragDropList->Compact		();
		for(tmp.y=0; tmp.y<=m_cellsCapacity.y-size.y; ++tmp.y )
			for(tmp.x=0; tmp.x<=m_cellsCapacity.x-size.x; ++tmp.x )
				if(IsRoomFree(tmp,size))
					return  tmp;
		R_ASSERT2		(0,"there are no free room to place item");
	}
	return			tmp;
}

bool CUICellContainer::HasFreeSpace		(const Ivector2& size){
	Ivector2 tmp;
	for(tmp.y=0; tmp.y<=m_cellsCapacity.y-size.y; ++tmp.y )
		for(tmp.x=0; tmp.x<=m_cellsCapacity.x-size.x; ++tmp.x )
			if(IsRoomFree(tmp,size))
				return true;

	return false;
}

bool CUICellContainer::IsRoomFree(const Ivector2& pos, const Ivector2& size)
{
	Ivector2 tmp;

	for(tmp.x =pos.x; tmp.x<pos.x+size.x; ++tmp.x)
		for(tmp.y =pos.y; tmp.y<pos.y+size.y; ++tmp.y)
		{
			if(!ValidCell(tmp))		return		false;

			CUICell& C				= GetCellAt(tmp);

			if(!C.Empty())			return		false;
		}
	return true;
}

void CUICellContainer::SetCellsCapacity(const Ivector2& c)
{
	m_cellsCapacity				= c;
	m_cells.resize				(c.x*c.y);
	ReinitSize					();
}

void CUICellContainer::SetCellSize(const Ivector2& new_sz)
{
	m_cellSize					= new_sz;
	ReinitSize					();
}

Ivector2 CUICellContainer::TopVisibleCell()
{
	return Ivector2().set	(0, iFloor(m_pParentDragDropList->ScrollPos()/float(CellSize().y)));
}

CUICell& CUICellContainer::GetCellAt(const Ivector2& pos)
{
	R_ASSERT			(ValidCell(pos));
	CUICell&	c		= m_cells[m_cellsCapacity.x*pos.y+pos.x];
	return				c;
}

Ivector2 CUICellContainer::GetItemPos(CUICellItem* itm)
{
	for(int x=0; x<m_cellsCapacity.x ;++x)
		for(int y=0; y<m_cellsCapacity.y ;++y){
			Ivector2 p;
			p.set(x,y);
		if(GetCellAt(p).m_item==itm)
			return p;
		}

		R_ASSERT(0);
		return Ivector2().set(-1,-1);
}

u32 CUICellContainer::GetCellsInRange(const Irect& rect, UI_CELLS_VEC& res)
{
	res.clear_not_free				();
	for(int x=rect.x1;x<=rect.x2;++x)
		for(int y=rect.y1;y<=rect.y2;++y)
			res.push_back	(GetCellAt(Ivector2().set(x,y)));

	std::unique				(res.begin(), res.end());
	return res.size			();
}

void CUICellContainer::ReinitSize()
{
	Fvector2							sz;
	sz.set								(CellSize().x*m_cellsCapacity.x, CellSize().y*m_cellsCapacity.y);
	SetWndSize							(sz);
	m_pParentDragDropList->ReinitScroll	();
}

void CUICellContainer::Grow()
{
	SetCellsCapacity	(Ivector2().set(m_cellsCapacity.x,m_cellsCapacity.y+1));
}

void CUICellContainer::Shrink()
{
}

bool CUICellContainer::ValidCell(const Ivector2& pos) const
{
	return !(pos.x<0 || pos.y<0 || pos.x>=m_cellsCapacity.x || pos.y>=m_cellsCapacity.y);
}

void CUICellContainer::ClearAll(bool bDestroy)
{
	{
		UI_CELLS_VEC_IT it		= m_cells.begin();
		UI_CELLS_VEC_IT it_e	= m_cells.end();
		for(;it!=it_e;++it)
			(*it).Clear();
	}
	while( !m_ChildWndList.empty() )
	{
		CUIWindow* w			= m_ChildWndList.back();
		CUICellItem* wc			= smart_cast<CUICellItem*>(w);
		VERIFY					(!wc->IsAutoDelete());
		DetachChild				(wc);	
		
		while( wc->ChildsCount() )
		{
			CUICellItem* ci		= wc->PopChild();
			R_ASSERT			(ci->ChildsCount()==0);

			if(bDestroy)
				delete_data		(ci);
		}
		
		if(bDestroy){
			delete_data			(wc);
		}
	}

}

Ivector2 CUICellContainer::PickCell(const Fvector2& abs_pos)
{
	Ivector2 res;
	Fvector2 ap;
	GetAbsolutePos							(ap);
	ap.sub									(abs_pos);
	ap.mul									(-1);
	res.x									= iFloor(ap.x/m_cellSize.x);
	res.y									= iFloor(ap.y/m_cellSize.y);
	if(!ValidCell(res))						res.set(-1, -1);
	return res;
}

void CUICellContainer::GetTexUVLT(Fvector2& uv, u32 col, u32 row,u8 select_mode)
{
	switch (select_mode)
	{   //select offset??? for drawing
		case 3: //suitable
		case 2: //selected
		case 1: //focused
			uv.set(0.50f,0.0f);
			break;
		case 0:
		default:
			uv.set(0.0f,0.0f);
			break;
	}
}

void CUICellContainer::Draw()
{
	Frect clientArea;
	m_pParentDragDropList->GetClientArea(clientArea);

	Ivector2			cell_cnt = m_pParentDragDropList->CellsCapacity();
	if					(cell_cnt.x==0 || cell_cnt.y==0)	return;

	Ivector2			cell_sz = CellSize();

	Irect				tgt_cells;
	tgt_cells.lt		= TopVisibleCell();
	tgt_cells.x2		= iFloor( (float(clientArea.width())+float(cell_sz.x)-EPS)/float(cell_sz.x)) + tgt_cells.lt.x;
	tgt_cells.y2		= iFloor( (float(clientArea.height())+float(cell_sz.y)-EPS)/float(cell_sz.y)) + tgt_cells.lt.y;

	clamp				(tgt_cells.x2, 0, cell_cnt.x-1);
	clamp				(tgt_cells.y2, 0, cell_cnt.y-1);

	Fvector2			lt_abs_pos;
	GetAbsolutePos		(lt_abs_pos);

	Fvector2					drawLT;
	drawLT.set					(lt_abs_pos.x+tgt_cells.lt.x*cell_sz.x, lt_abs_pos.y+tgt_cells.lt.y*cell_sz.y);
	UI()->ClientToScreenScaled	(drawLT, drawLT.x, drawLT.y);

	const Fvector2 pts[6] =		{{0.0f,0.0f},{1.0f,0.0f},{1.0f,1.0f},
								 {0.0f,0.0f},{1.0f,1.0f},{0.0f,1.0f}};
#define ty 1.0f
#define tx 0.5f
	const Fvector2 uvs[6] =		{{0.0f,0.0f},{tx,0.0f},{tx,ty},
								 {0.0f,0.0f},{tx,ty},{0.0f,ty}};

	// calculate cell size in screen pixels
	Fvector2 f_len;
	UI()->ClientToScreenScaled(f_len, float(cell_sz.x), float(cell_sz.y) );

	// fill cell buffer
	u32 vOffset					= 0;
	FVF::TL* start_pv			= static_cast<FVF::TL*>(RCache.Vertex.Lock((tgt_cells.width() + 1) * (tgt_cells.height() + 1) * 6, hGeom.stride(), vOffset));
	FVF::TL* pv					= start_pv;
	for (int x=0; x<=tgt_cells.width(); ++x){
		for (int y=0; y<=tgt_cells.height(); ++y){
			u8 select_mode = 0;

			Ivector2 cpos;
			cpos.set( x, y );
			cpos.add( TopVisibleCell() );
			CUICell& ui_cell = GetCellAt( cpos );
			u32 back_color=0xFFFFFFFF;
			if (!ui_cell.Empty() )
			{
				if ( ui_cell.m_item->m_focused && g_uCommonFlags.test(E_COMMON_FLAGS::uiShowFocused))
				{
					back_color=m_focused_color.get();
					select_mode = back_color==0xFFFFFFFF ? 0 : 1;
					//ui_cell.m_item->SetClrLightAnim(nullptr, true, false, false, true);
				} 
				else if ( ui_cell.m_item->m_selected && g_uCommonFlags.test(E_COMMON_FLAGS::uiShowSelected))
				{
					back_color=m_selected_color.get();
					select_mode = back_color==0xFFFFFFFF ? 0 : 2;
					//ui_cell.m_item->SetClrLightAnim(nullptr, true, false, false, true);
				}
				else if ( ui_cell.m_item->m_suitable )
				{
					back_color=m_suitable_color.get();
					select_mode = back_color==0xFFFFFFFF ? 0 : 3;
					if (!ui_cell.m_item->IsAnimStarted())
					{
						ui_cell.m_item->SaveColors();
						if (m_anim_mSec>0)
						{
							ui_cell.m_item->ResetClrAnimation();	
							ui_cell.m_item->SetClrAnimLength(m_anim_mSec);
							ui_cell.m_item->SetClrLightAnim("ui_slow_blinking", false, false, false, true);
						}
					}
				}
			}
			Fvector2			tp;
			GetTexUVLT			(tp,tgt_cells.x1+x,tgt_cells.y1+y,select_mode);
			for (u32 k=0; k<6; ++k,++pv){
				const Fvector2& p	= pts[k];
				const Fvector2& uv	= uvs[k];
				pv->set			(iFloor(drawLT.x + p.x*(f_len.x) + f_len.x*x)-0.5f, 
								 iFloor(drawLT.y + p.y*(f_len.y) + f_len.y*y)-0.5f, 
								 back_color,
								 tp.x+uv.x,tp.y+uv.y);
			}
		}
	}
	std::ptrdiff_t p_cnt		= (pv-start_pv)/3;
	RCache.Vertex.Unlock		(u32(pv-start_pv),hGeom.stride());

	UI()->PushScissor					(clientArea);

	if (p_cnt!=0){
		// draw grid
		RCache.set_Shader		(hShader);
		RCache.set_Geometry		(hGeom);
		RCache.Render			(D3DPT_TRIANGLELIST,vOffset,u32(p_cnt));
	}

	//draw shown items in range
	if( GetCellsInRange(tgt_cells,m_cells_to_draw) ){
		UI_CELLS_VEC_IT it = m_cells_to_draw.begin();
		for(;it!=m_cells_to_draw.end();++it)
		{
			CUICell& cell = (*it);
			if( !cell.Empty() && !cell.m_item->m_b_already_drawn /* (cell.m_item->m_drawn_frame != Device.dwFrame)*/)
			{
				cell.m_item->Draw				();
			}
		}
	}

	UI()->PopScissor			();
}