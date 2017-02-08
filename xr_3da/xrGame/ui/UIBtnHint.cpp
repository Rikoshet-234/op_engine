#include "stdafx.h"
#include "UIBtnHint.h"
#include "UIFrameLineWnd.h"
#include "UIXmlInit.h"
#include "UILines.h"
#include "UILine.h"

CUIButtonHint*		g_btnHint = nullptr;

CUIButtonHint::CUIButtonHint	():m_ownerWnd(nullptr),m_enabledOnFrame(false)
{
	Device.seqRender.Add		(this, REG_PRIORITY_LOW-1000);

	CUIXmlInit					xml_init;
	CUIXml						uiXml;
	bool xml_result				= uiXml.Init(CONFIG_PATH, UI_PATH, "hint_item.xml");
	R_ASSERT3					(xml_result, "xml file not found", "hint_item.xml");

	xml_init.InitWindow			(uiXml,"button_hint",0,this);
	
	m_border					= xr_new<CUIFrameWindow>();
	m_border->SetAutoDelete(true);
	CUIWindow::AttachChild					(m_border);
	xml_init.InitFrameWindow		(uiXml,"button_hint:frame_line",0,m_border);

	m_text						= xr_new<CUIStatic>();
	m_text->SetAutoDelete(true);
	CUIWindow::AttachChild					(m_text);
	xml_init.InitStatic			(uiXml,"button_hint:description",0,m_text);


}

CUIButtonHint::~CUIButtonHint	()
{
	Device.seqRender.Remove		(this);
}

void CUIButtonHint::Discard()
{
	m_ownerWnd = nullptr;
}

void CUIButtonHint::OnRender	()
{
	if(m_enabledOnFrame){
		m_text->Update		();
		m_border->GetTitleStatic()->SetStretchTexture(true);
		m_border->Update	();
		m_border->SetColor	(color_rgba(255,255,255,color_get_A(m_text->GetTextColor())));
		Draw				();
		m_enabledOnFrame	= false;
	}
}

void CUIButtonHint::SetHintText	(CUIWindow* w, LPCSTR text)
{
	m_ownerWnd= w;
	m_text->SetText(text);
	m_text->AdjustHeightToText();
	//m_text->AdjustWidthToText();
	Fvector2 new_size;

	new_size.x				= m_text->GetWndPos().x+m_text->GetWndSize().x+20.0f;
	new_size.y				= m_text->GetWndPos().y+m_text->GetWndSize().y+40.0f;
	//m_text->SetWndSize(new_size);
	//new_size.x					= GetWndSize().x;
	//new_size.y					= m_text->GetWndSize().y+20.0f;

	m_border->SetWndSize(new_size);
	m_text->SetWndSize(new_size);
	SetWndSize(new_size);
	m_text->ResetClrAnimation();
}
