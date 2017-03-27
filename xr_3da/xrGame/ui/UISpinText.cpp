#include "StdAfx.h"
#include "UISpinText.h"
#include "UILines.h"
#include "../string_table.h"
#include "UIFrameLineWnd.h"
#include "UI3tButton.h"




CUISpinText::CUISpinText(){
	m_curItem = -1;
}

void CUISpinText::AddItem_(const char* item, int id)
{
	SInfo			_info;
	_info._orig		= item;
	_info._transl	= CStringTable().translate(item);
	_info._id		= id;

	m_list.push_back( _info );
	if (-1 == m_curItem)
	{
		m_curItem		= 0;
		SetItem			();
	}
}

void CUISpinText::SetItem()
{
	R_ASSERT			(m_curItem != -1);
	m_pLines->SetText	(m_list[m_curItem]._transl.c_str());
}

LPCSTR CUISpinText::GetTokenText()
{
	R_ASSERT			(m_curItem != -1);
	return				m_list[m_curItem]._orig.c_str();
}

void CUISpinText::SetCurrentValue(){
	xr_token* tok = GetOptToken();

	while (tok->name){
		AddItem_(tok->name, tok->id);
		tok++;
	}
	xr_string val = GetOptTokenValue();

	for (u32 i = 0; i < m_list.size(); i++)
		if (val == m_list[i]._orig.c_str())
		{
			m_curItem	= i;
			break;
		}

	SetItem();
}

void CUISpinText::SaveValue()
{
	CUIOptionsItem::SaveValue		();
	SaveOptTokenValue				(m_list[m_curItem]._orig.c_str());
}

bool CUISpinText::IsChanged()
{
	return 0 != xr_strcmp(GetOptTokenValue(), m_list[m_curItem]._orig.c_str());
}

void CUISpinText::OnBtnUpClick()
{
	if (CanPressUp())
	{
		m_curItem		++;
		SetItem			();
	}

	CUICustomSpin::OnBtnUpClick();
}

void CUISpinText::OnBtnDownClick()
{
	if (CanPressDown())
	{
		m_curItem--;
		SetItem		();
	}

	CUICustomSpin::OnBtnDownClick();
}

bool CUISpinText::CanPressUp()
{
	return m_curItem < (int)m_list.size() - 1;
}

bool CUISpinText::CanPressDown()
{
	return m_curItem > 0;
}

CUISpinTextCustom::CUISpinTextCustom()
{
	m_backupIndex=-1;

}

CUISpinTextCustom::~CUISpinTextCustom()
{
	m_pButtonsSpinClickCallback.clear();
}

void CUISpinTextCustom::SetCurrentValue()
{
	
}

void CUISpinTextCustom::SaveValue()
{
	m_backupIndex=m_curItem;
}

bool CUISpinTextCustom::IsChanged()
{
	return m_backupIndex!=m_curItem;
}

void CUISpinTextCustom::SetSpinButtonCallback(const luabind::functor<void> &functor)
{
	if (functor.is_valid())
		m_pButtonsSpinClickCallback.set(functor);
	else
		m_pButtonsSpinClickCallback.clear();
}

void CUISpinTextCustom::SetItem()
{
	if (m_curItem==-1)
		m_pLines->SetText("");
	else
		m_pLines->SetText(CStringTable().translate(m_list[m_curItem]._orig).c_str());
}

int CUISpinTextCustom::GetIdByIndex(int index)
{
	if (index > static_cast<int>(m_list.size())-1 || index < 0)
		return -1;
	return m_list[index]._id;
}

LPCSTR CUISpinTextCustom::GetTextByIndex(int index)
{
	if (index > static_cast<int>(m_list.size()) - 1 || index < 0)
		return "";
	return m_list[index]._orig.c_str();
}

int CUISpinTextCustom::GetSelectedId()
{
	return m_list[m_curItem]._id;
}

void CUISpinTextCustom::SetSelectedId(int id)
{
	Items_it st=std::find_if(m_list.begin(),m_list.end(),[&](SInfo info)
	{
		return info._id==id;
	});
	if (st!=m_list.end())
	{
		m_curItem=std::distance(m_list.begin(),st);
		SetItem();
	}
	else
	{
		m_curItem=-1;
		m_pLines->SetText("");
	}
}

LPCSTR CUISpinTextCustom::GetSelectedText()
{
	return m_list[m_curItem]._orig.c_str();
}

void CUISpinTextCustom::SetSelectedText(LPCSTR text)
{
	Items_it st=std::find_if(m_list.begin(),m_list.end(),[&](SInfo info)
	{
		return xr_strcmp(info._orig.c_str(),text)==0;
	});
	if (st!=m_list.end())
	{
		m_curItem=std::distance(m_list.begin(),st);
		SetItem();
	}
	else
	{
		m_curItem=-1;
		m_pLines->SetText("");
	}
}

void CUISpinTextCustom::AddItem(const char* text, int id)
{
	SInfo			_info;
	_info._orig		= text;
	_info._id		= id;

	m_list.push_back( _info );
	if (-1 == m_curItem)
	{
		m_curItem		= 0;
		m_pLines->SetText(CStringTable().translate(m_list[m_curItem]._orig).c_str());
	}
}

void CUISpinTextCustom::SetFont(CGameFont* pFont)
{
	if (pFont!=nullptr)
	{
		CUISpinText::SetFont(pFont);
		m_pLines->SetFont(pFont);
	}
}

void CUISpinTextCustom::OnBtnUpClick()
{
	int old_index = m_curItem;
	CUISpinText::OnBtnUpClick();
	int new_index = m_curItem;
	m_pButtonsSpinClickCallback(old_index, new_index);
}

void CUISpinTextCustom::OnBtnDownClick()
{
	int old_index = m_curItem;
	CUISpinText::OnBtnDownClick();
	int new_index = m_curItem;
	m_pButtonsSpinClickCallback(old_index, new_index);
}

