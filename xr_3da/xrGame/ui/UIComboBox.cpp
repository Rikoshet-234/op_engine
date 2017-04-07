// File:        UIComboBox.cpp
// Description: guess :)
// Created:     10.12.2004
// Author:      Serhiy O. Vynnychenko
// Mail:        narrator@gsc-game.kiev.ua
// 
// Copyright 2004 GSC Game World
//

#include "StdAfx.h"
#include "UIComboBox.h"
#include "UITextureMaster.h"
#include "UIScrollBar.h"
#include "UIListBoxItem.h"

#define CB_HEIGHT 23.0f
#define BTN_SIZE  23.0f

CUIComboBox::CUIComboBox()
{
	CUIWindow::AttachChild			(&m_frameLine);
	CUIWindow::AttachChild			(&m_text);

//.	AttachChild			(&m_btn);

	CUIWindow::AttachChild			(&m_frameWnd);
	CUIWindow::AttachChild			(&m_list);
	m_fItemHeight=0;
	m_iListHeight		= 0;
	m_bInited			= false;
	m_eState			= LIST_FONDED;

	m_textColor[0]		= 0xff00ff00;
}


void CUIComboBox::Init(float x, float y, float width){
	m_bInited = true;
	if (0 == m_iListHeight)
		m_iListHeight = 4;

//.	width								-= BTN_SIZE;

	CUIWindow::Init						(x, y, width, CB_HEIGHT);
	// Frame Line
	m_frameLine.Init					(0, 0, width, CB_HEIGHT);
	m_frameLine.InitEnabledState		("ui_cb_linetext_e"); // horizontal by default
	m_frameLine.InitHighlightedState	("ui_cb_linetext_h");


	// Edit Box on left side of frame line
	m_text.Init							(0, 0, width, CB_HEIGHT); 
	m_text.SetTextColor					(m_textColor[0]);
	m_text.Enable						(false);
	// Button on right side of frame line
//.	m_btn.Init							("ui_cb_button", width, 0, BTN_SIZE, BTN_SIZE);


	// height of list equal to height of ONE element
	m_fItemHeight					= CUITextureMaster::GetTextureHeight("ui_cb_listline_b");
	m_list.Init							(0, CB_HEIGHT, width, m_fItemHeight*m_iListHeight);
	m_list.Init							();
	m_list.SetTextColor					(m_textColor[0]);
	m_list.SetSelectionTexture			("ui_cb_listline");
	m_list.SetItemHeight				(m_fItemHeight);
	// frame(texture) for list
	m_frameWnd.Init						(0,  CB_HEIGHT, width, m_list.GetItemHeight()*m_iListHeight);
	m_frameWnd.InitTexture				("ui_cb_listbox");

	m_list.Show							(false);
	m_frameWnd.Show						(false);
}
void CUIComboBox::Init(float x, float y, float width, float height)
{
	this->Init		(x, y, width);
}
CUIComboBox::~CUIComboBox()
{}

void CUIComboBox::SetListLength(int length){
	R_ASSERT(0 == m_iListHeight);
	m_iListHeight = length;
}


#include "uilistboxitem.h"
CUIListBoxItem* CUIComboBox::AddItem_(LPCSTR str, int _data)
{
	R_ASSERT2			(m_bInited, "Can't add item to ComboBox before Initialization");
	CUIListBoxItem* itm = m_list.AddItem(str);
	itm->SetData		((void*)(__int64)_data);
	return				itm;
}


void CUIComboBox::OnListItemSelect()
{
	m_text.SetText			(m_list.GetSelectedText());    
	CUIListBoxItem* itm		= m_list.GetSelectedItem();
	
	int bk_itoken_id		= m_itoken_id;
	
	m_itoken_id				= (int)(__int64)itm->GetData();
	ShowList				(false);

	if(bk_itoken_id!=m_itoken_id)
	{
		SaveValue		();
		GetMessageTarget()->SendMessage(this, LIST_ITEM_SELECT, NULL);
	}
}

#include "../string_table.h"
void CUIComboBox::SetCurrentValue()
{
	m_list.Clear		();
	xr_token* tok		= GetOptToken();

	bool fp_init = m_entry == "font_profile";
	while (tok->name)
	{		
		LPCSTR txt = tok->name;
		if (fp_init)
			txt = *CStringTable().translate(pSettings->r_string("font_profiles", txt));
		if (xr_strlen(txt)>0)
			AddItem_(txt, tok->id);
		tok++;
	}

	LPCSTR cur_val		= *CStringTable().translate( GetOptTokenValue());
	m_text.SetText		( cur_val );
	m_list.SetSelectedText( cur_val );
	
	CUIListBoxItem* itm	= m_list.GetSelectedItem();
	if(itm)
		m_itoken_id			= (int)(__int64)itm->GetData();
	else
		m_itoken_id			= 1; //first
}

void CUIComboBox::SaveValue()
{
	CUIOptionsItem::SaveValue	();
	xr_token* tok				= GetOptToken();
	LPCSTR	cur_val				= get_token_name(tok, m_itoken_id);
	SaveOptTokenValue			(cur_val);
}

bool CUIComboBox::IsChanged()
{
	return				(m_backup_itoken_id != m_itoken_id);
/*
	xr_token* tok		= GetOptToken();
	LPCSTR	cur_val		= get_token_name(tok, m_itoken_id);

	bool bChanged		= (0 != xr_strcmp(GetOptTokenValue(), cur_val));

	return				bChanged;
*/
}

LPCSTR CUIComboBox::GetText()
{
	return m_text.GetText	();
}

void CUIComboBox::SetItem(int idx)
{
	m_list.SetSelectedIDX	(idx);
	CUIListBoxItem* itm		= m_list.GetSelectedItem();
	m_itoken_id				= (int)(__int64)itm->GetData();

	m_text.SetText			(m_list.GetSelectedText());
	
}
void CUIComboBox::OnBtnClicked()
{
	ShowList				(!m_list.IsShown());
}

void CUIComboBox::ShowList(bool bShow)
{
	if (bShow)
	{
		SetHeight			(m_text.GetHeight() + m_list.GetHeight());

		m_list.Show			(true);
		m_frameWnd.Show		(true);

		m_eState			= LIST_EXPANDED;
		GetParent()->SetCapture(this, true);
	}
	else
	{
		m_list.Show			(false);
		m_frameWnd.Show		(false);
		SetHeight			(m_frameLine.GetHeight());
		GetParent()->SetCapture(this, false);

		m_eState			= LIST_FONDED;
	}
}

CUIListBox* CUIComboBox::GetListWnd()
{
	return &m_list;
}
void CUIComboBox::Update()
{
	CUIWindow::Update	();
	if (!m_bIsEnabled)
	{
		SetState		(S_Disabled);
		m_text.SetTextColor(m_textColor[1]);
	}
	else
		m_text.SetTextColor(m_textColor[0]);

}

void CUIComboBox::OnFocusLost()
{
	CUIWindow::OnFocusLost();
	if (m_bIsEnabled)
		SetState(S_Enabled);

}

void CUIComboBox::OnFocusReceive()
{
	CUIWindow::OnFocusReceive();
	if (m_bIsEnabled)
		SetState(S_Highlighted);
}

bool CUIComboBox::OnMouse(float x, float y, EUIMessages mouse_action){
	if(CUIWindow::OnMouse(x, y, mouse_action)) 
		return true;

	bool bCursorOverScb = false;
//.	bCursorOverScb |= (0 <= x) && (GetWidth() >= x) && (0 <= y) && (GetHeight() >= y);

//.	Frect wndRect		= m_list.ScrollBar()->GetWndRect();
//.	bCursorOverScb		= wndRect.in(m_list.ScrollBar()->cursor_pos)
	bCursorOverScb		= m_list.ScrollBar()->CursorOverWindow();
	switch (m_eState){
		case LIST_EXPANDED:			

			if (  (!bCursorOverScb) &&  mouse_action == WINDOW_LBUTTON_DOWN)
			{
				ShowList(false);
				return true;
			}
			break;
		case LIST_FONDED:
			if(mouse_action==WINDOW_LBUTTON_DOWN)
			{
				OnBtnClicked();
				return true;			
			}break;
		default:
			break;
	}	
	 

		return false;
}

void CUIComboBox::SetState(UIState state)
{
	m_frameLine.SetState	(state);
}

void CUIComboBox::SendMessage(CUIWindow* pWnd, s16 msg, void* pData)
{
	CUIWindow::SendMessage	(pWnd, msg, pData);

	switch (msg){
/*		case BUTTON_CLICKED:
			if (pWnd == &m_btn || pWnd == &m_list)
				OnBtnClicked();
			break;
*/
		case LIST_ITEM_CLICKED:
			if (pWnd == &m_list)
				OnListItemSelect();	
			break;
		default:
			break;
	}
}

void CUIComboBox::SeveBackUpValue()
{
	m_backup_itoken_id = m_itoken_id;
}

void CUIComboBox::Undo()
{
	SetItem				(m_backup_itoken_id);
	SaveValue			();
	SetCurrentValue		();
}

void CUIComboBoxCustom::RecalcListHeight()
{
	m_pItemsWnd->SetHeight(m_iListItemsSize*m_fItemHeight);
}

CUIComboBoxCustom::CUIComboBoxCustom()
{
	m_iListItemsSize=4;
	m_fItemHeight=1;
	m_pOwner=this;

	m_pTextBox=xr_new<CUIStatic>();
	m_pTextBox->SetAutoDelete(true);
	CUIWindow::AttachChild(m_pTextBox);

	m_pExpandButton=xr_new<CUIButton>();
	m_pExpandButton->SetAutoDelete(true);
	CUIWindow::AttachChild(m_pExpandButton);

	m_pItemsWnd=xr_new<CUIStatic>();
	m_pItemsWnd->SetAutoDelete(true);
	CUIWindow::AttachChild(m_pItemsWnd);

	m_pItemsWnd->Show(false);

	m_pItemList=xr_new<CUIListBox>();
	m_pItemsWnd->SetAutoDelete(true);
	//m_pItemsWnd->AttachChild(m_pItemList);
}

CUIComboBoxCustom::~CUIComboBoxCustom()
{
	xr_free(m_pTextBox);
	xr_free(m_pExpandButton);
	xr_free(m_pItemList);
	xr_free(m_pItemsWnd);	
}

void CUIComboBoxCustom::Init(float x, float y, float width, float height)
{
	CUIStatic::Init(x, y, width, height);
}

void CUIComboBoxCustom::Init()
{
	SetWndPos(m_pTextBox->GetWndPos());
	SetWndSize(m_pTextBox->GetWndSize());
}

void CUIComboBoxCustom::SendMessageA(CUIWindow* pWnd, s16 msg, void* pData)
{
	CUIStatic::SendMessage	(pWnd, msg, pData);
	switch (msg){
		case BUTTON_CLICKED:
			{
				if (pWnd == m_pExpandButton)
					if (!m_pItemsWnd->GetVisible())
					{
						m_fBackup=GetHeight();
						m_pItemsWnd->SetWndPos(Fvector2().set(0.f,m_fItemHeight+5));
						m_pItemsWnd->SetWndSize(Fvector2().set(300.f,200.f));
						SetHeight(m_pTextBox->GetHeight() + m_pItemsWnd->GetHeight()+5);
						//m_pOwner->BringToTop(this);
						//m_pOwner->SetCapture(this, true);
						//GetParent()->GetParent()->GetParent()->SetCapture(this, true);
						//GetParent()->GetParent()->GetParent()->AttachChild(m_pItemsWnd);
						//GetParent()->GetParent()->GetParent()->BringToTop(m_pItemsWnd);
						GetParent()->SetCapture(this, true);
						//GetParent()->AttachChild(m_pItemsWnd);
						//GetParent()->BringToTop(m_pItemsWnd);
						m_pItemsWnd->Show(true);
						//GetParent()->GetParent()->GetParent()->BringToTop(m_pItemsList);
					}
					else
					{
						SetHeight(m_fBackup);
						GetParent()->SetCapture(this, false);
						m_pItemsWnd->Show(false);
					}
			}	
			break;

		case LIST_ITEM_CLICKED:
			/*if (pWnd == &m_list)
				OnListItemSelect();	*/
			break;
		default:
			break;
	}
}

void CUIComboBoxCustom::SetListLength(int length)
{
	m_iListItemsSize = length;
	RecalcListHeight();
	
}

void CUIComboBoxCustom::SetitemHeight(float height)
{
	m_fItemHeight=height;
	RecalcListHeight();
}

void CUIComboBoxCustom::SetOwner(CUIWindow* owner)
{
	//if (owner!=m_pOwner && owner)
	//{
	//	m_pOwner=owner;
	//}
	if (owner!=m_pOwner && owner)
	{
		CUIScrollView* new_owner = smart_cast<CUIScrollView*>(owner);
		CUIScrollView* current_owner = smart_cast<CUIScrollView*>(m_pOwner);	
		if (current_owner)
			current_owner->RemoveWindow(m_pItemsWnd);
		else
			m_pOwner->DetachChild(m_pItemsWnd);
		m_pOwner=owner;
		if(new_owner)
			new_owner->AddWindow(m_pItemsWnd, true);
		else
			m_pOwner->AttachChild(m_pItemsWnd);
	}
}

//CUIListBoxItem* CUIComboBoxCustom::AddItem(LPCSTR str, int _data)
//{
//	return AddItem_(str,_data);
//}
