#include "stdafx.h"
#include "UICellItem.h"
#include "../xr_level_controller.h"
#include "../../xr_input.h"
#include "../HUDManager.h"
#include "../level.h"
#include "../object_broker.h"
#include "xrUIXmlParser.h"
#include "UIXmlInit.h"
#include "UIDragDropListEx.h"
#include "../Weapon.h"
#include "../ExoOutfit.h"

#define CELL_ITEM_XML "cell_item.xml"

CUICellItem::CUICellItem()
{
	m_pParentList		= nullptr;
	m_pData				= nullptr;
	m_custom_draw		= nullptr;
	m_b_already_drawn	= false;
	m_drawn_frame=0;
	SetAccelerator		(0);
	m_b_destroy_childs	= true;
	m_focused=false;
	m_selected=false;
	m_suitable=false;
	p_ConditionProgressBar=nullptr;
	init();
}

CUICellItem::~CUICellItem()
{

	if(m_b_destroy_childs)
		delete_data	(m_childs);

	delete_data		(m_custom_draw);
}


void CUICellItem::Draw()
{
	m_drawn_frame=Device.dwFrame;
	m_b_already_drawn		= true;
	inherited::Draw();
	if(m_custom_draw) 
		m_custom_draw->OnDraw(this);
};

bool CUICellItem::OnMouse(float x, float y, EUIMessages mouse_action)
{
	if(mouse_action == WINDOW_LBUTTON_DOWN){
		GetMessageTarget()->SendMessage(this, DRAG_DROP_ITEM_SELECTED, nullptr);
		return false;
	}else
	if(mouse_action == WINDOW_MOUSE_MOVE && pInput->iGetAsyncBtnState(0)){
		GetMessageTarget()->SendMessage(this, DRAG_DROP_ITEM_DRAG, nullptr);
		return true;
	}else
	if(mouse_action==WINDOW_LBUTTON_DB_CLICK){
		GetMessageTarget()->SendMessage(this, DRAG_DROP_ITEM_DB_CLICK, nullptr);
		return true;
	}else
	if(mouse_action==WINDOW_RBUTTON_DOWN){
		GetMessageTarget()->SendMessage(this, DRAG_DROP_ITEM_RBUTTON_CLICK, nullptr);
		return true;
	}
	
	return false;
};

bool CUICellItem::OnKeyboard(int dik, EUIMessages keyboard_action)
{
	if (WINDOW_KEY_PRESSED == keyboard_action)
	{
		if (GetAccelerator() == dik)
		{
			GetMessageTarget()->SendMessage(this, DRAG_DROP_ITEM_DB_CLICK, nullptr);
			return		true;
		}
	}
	return inherited::OnKeyboard(dik, keyboard_action);
}

CUIDragItem* CUICellItem::CreateDragItem()
{
	CUIDragItem* tmp;
	tmp = xr_new<CUIDragItem>(this);
	Frect r;
	GetAbsoluteRect(r);
	tmp->Init(GetShader(),r,GetUIStaticItem().GetOriginalRect());
	return tmp;
}

void CUICellItem::SetOwnerList(CUIDragDropListEx* p)	
{
	m_pParentList=p;
	updateConditionBar();
}

bool CUICellItem::EqualTo(CUICellItem* itm)
{
	return (m_grid_size.x==itm->GetGridSize().x) && (m_grid_size.y==itm->GetGridSize().y);
}

u32 CUICellItem::ChildsCount()
{
	return m_childs.size();
}

void CUICellItem::PushChild(CUICellItem* c)
{
	R_ASSERT(c->ChildsCount()==0);
	VERIFY				(this!=c);
	m_childs.push_back	(c);
	UpdateItemText		();
}

CUICellItem* CUICellItem::PopChild()
{
	CUICellItem* itm	= m_childs.back();
	m_childs.pop_back	();
	std::swap			(itm->m_pData, m_pData);
	UpdateItemText		();
	R_ASSERT			(itm->ChildsCount()==0);
	itm->SetOwnerList	(nullptr);
	return				itm;
}

bool CUICellItem::HasChild(CUICellItem* item)
{
	return (m_childs.end() != std::find(m_childs.begin(), m_childs.end(), item) );
}

void CUICellItem::UpdateItemText()
{
	string32			str;
	if(ChildsCount())
		sprintf_s				(str,"x%d",ChildsCount()+1);
	else
		sprintf_s				(str,"");

	SetText				(str);
}

void CUICellItem::init()
{
	CUIXml	uiXml;
	uiXml.Init( CONFIG_PATH, UI_PATH, CELL_ITEM_XML );
	CUIXmlInit							xml_init;
	p_ConditionProgressBar=xr_new<CUIProgressBar>();
	p_ConditionProgressBar->SetAutoDelete(true);
	AttachChild (p_ConditionProgressBar);
	xml_init.InitProgressBar (uiXml, "progress_item_condition", 0, p_ConditionProgressBar);
	p_ConditionProgressBar->Show(true);
}

void CUICellItem::updateConditionBar()
{
	if (m_pParentList && m_pParentList->GetShowConditionBar())
	{
		PIItem itm = static_cast<PIItem>(m_pData);
		CWeapon* pWeapon = smart_cast<CWeapon*>(itm);
		CCustomOutfit* pOutfit=smart_cast<CCustomOutfit*>(itm);
		if (pWeapon || pOutfit)
		{
			Ivector2 itm_grid_size = GetGridSize();
			Ivector2 cell_size = m_pParentList->CellSize();
			float x = 1.f;
			float y = itm_grid_size.y * cell_size.y - p_ConditionProgressBar->GetHeight()-1;

			p_ConditionProgressBar->SetWndPos(Fvector2().set(x,y));
			//p_ConditionProgressBar->SetWidth(float(itm_grid_size.x*cell_size.x-5));
			p_ConditionProgressBar->SetProgressPos(itm->GetCondition()*100+1.0f-EPS);
			p_ConditionProgressBar->Show(true);
			return;
		}
	}
	p_ConditionProgressBar->Show(false);
}


void CUICellItem::SetCustomDraw			(ICustomDrawCell* c){
	if (m_custom_draw)
		xr_delete(m_custom_draw);
	m_custom_draw = c;
}

void CUICellItem::SaveColors()
{
	m_preAnimTexColor.set(GetTextureColor());
	m_preAnimTextColor.set(GetTextColor());
}

void CUICellItem::RestoreColors()
{
	SetTextureColor(m_preAnimTexColor.get());
	SetTextColor(m_preAnimTextColor.get());
}

CUIDragItem::CUIDragItem(CUICellItem* parent)
{
	m_back_list						= nullptr;
	m_pParent						= parent;
	CUIWindow::AttachChild						(&m_static);
	Device.seqRender.Add			(this, REG_PRIORITY_LOW-5000);
	Device.seqFrame.Add				(this, REG_PRIORITY_LOW-5000);
	VERIFY							(m_pParent->GetMessageTarget());
}

CUIDragItem::~CUIDragItem()
{
	Device.seqRender.Remove			(this);
	Device.seqFrame.Remove			(this);
}

void CUIDragItem::Init(const ref_shader& sh, const Frect& rect, const Frect& text_rect)
{
	SetWndRect						(rect);
	m_static.SetShader				(sh);
	m_static.SetOriginalRect		(text_rect);
	m_static.SetWndPos				(0.0f,0.0f);
	m_static.SetWndSize				(GetWndSize());
	m_static.TextureAvailable		(true);
	m_static.TextureOn				();
	m_static.SetColor				(color_rgba(255,255,255,170));
	m_static.SetStretchTexture		(true);
	m_pos_offset.sub				(rect.lt, GetUICursor()->GetCursorPosition());
}

bool CUIDragItem::OnMouse(float x, float y, EUIMessages mouse_action)
{
	if(mouse_action == WINDOW_LBUTTON_UP)
	{
		m_pParent->GetMessageTarget()->SendMessage(m_pParent,DRAG_DROP_ITEM_DROP,nullptr);
		return true;
	}
	return false;
}

void CUIDragItem::OnRender()
{
	Draw			();
}

void CUIDragItem::OnFrame()
{
	Update			();
}

void CUIDragItem::Draw()
{
	Fvector2 tmp;
	tmp.sub					(GetWndPos(), GetUICursor()->GetCursorPosition());
	tmp.sub					(m_pos_offset);
	tmp.mul					(-1.0f);
	MoveWndDelta			(tmp);
	UI()->PushScissor		(UI()->ScreenRect(),true);

	inherited::Draw();

	UI()->PopScissor();
}

void CUIDragItem::SetBackList(CUIDragDropListEx*l)
{
	if(m_back_list!=l){
		m_back_list=l;
	}
}

Fvector2 CUIDragItem::GetPosition()
{
	return Fvector2().add(m_pos_offset, GetUICursor()->GetCursorPosition());
}

