#include "StdAfx.h"
#include "UIArtefactPanel.h"
#include "UIInventoryUtilities.h"
#include "UIXmlInit.h"


#include "../artifact.h"

using namespace InventoryUtilities;

CUIArtefactPanel::CUIArtefactPanel(): m_fScale(1), m_eIconsAligment(EAligment::alLeft)
{
	m_sArtPanel = xr_new<CUIStatic>();
	m_sArtPanel->SetAutoDelete(false);
	m_sArtPanel->SetVisible(false);
	CUIWindow::AttachChild(m_sArtPanel);
}

CUIArtefactPanel::~CUIArtefactPanel()
{
	xr_delete(m_sArtPanel);
}

void CUIArtefactPanel::InitFromXML	(CUIXml& xml, LPCSTR path, int index)
{
	CUIXmlInit::InitWindow		(xml, path, index, this);
	m_cell_size.x				= xml.ReadAttribFlt(path, index, "cell_width");
	m_cell_size.y				= xml.ReadAttribFlt(path, index, "cell_height");
	m_fScale					= xml.ReadAttribFlt(path, index, "scale");
	xr_string al = xml.ReadAttrib(path, index, "icons_align","r");
	if (0 == xr_strcmp(al.c_str(), "c"))
		m_eIconsAligment = EAligment::alCenter;
	else if (0 == xr_strcmp(al.c_str(), "r"))
		m_eIconsAligment = EAligment::alRight;
	else if (0 == xr_strcmp(al.c_str(), "l"))
		m_eIconsAligment = EAligment::alLeft;
}

void CUIArtefactPanel::InitIcons(const xr_vector<const CArtefact*>& artefacts)
{
#if 0
	m_si.SetShader(GetEquipmentIconsShader());
	m_vRects.clear();

	for (xr_vector<const CArtefact*>::const_iterator it = artefacts.begin(); it != artefacts.end(); ++it)
	{
		const CArtefact* artefact = *it;
		Frect rect = artefact->GetIconInfo().getOriginalRect();
		m_vRects.push_back(rect);
	}
#endif

	m_sArtPanel->DetachAll();
	float x=0.0f;
	float y=0.0f;
	float iIndent = 1.0f;
	for (xr_vector<const CArtefact*>::const_iterator it = artefacts.begin(); it != artefacts.end(); ++it)
	{
		const CArtefact* artefact = *it;
		Frect rect = artefact->GetIconInfo().getOriginalRect();
		float iHeight = m_fScale*(rect.bottom - rect.top);
		float iWidth = (m_cell_size.x / m_cell_size.y)*m_fScale*(rect.right - rect.left);
		CUIStatic* art = xr_new<CUIStatic>();
		art->SetAutoDelete(true);
		art->SetShader(InventoryUtilities::GetEquipmentIconsShader());
		art->SetOriginalRect(rect.left, rect.top, rect.width(), rect.height());
		art->SetWndRect(x, y, iWidth, iHeight);
		art->SetStretchTexture(true);
		m_sArtPanel->AttachChild(art);
		x = x + iIndent + iWidth;
	}

	if (artefacts.size() > 0)
	{
		m_sArtPanel->SetWidth(x + iIndent);
		Frect rect = GetWndRect();
		if (m_eIconsAligment == EAligment::alLeft)
		{
			m_sArtPanel->SetWndPos(0,0);
		}
		else if (m_eIconsAligment == EAligment::alRight)
		{
			m_sArtPanel->SetWndPos(GetWidth() - m_sArtPanel->GetWidth() - iIndent, 0);
		}
		else if (m_eIconsAligment == EAligment::alCenter)
		{
			float mc = GetWidth() / 2;
			float ac = m_sArtPanel->GetWidth() / 2;
			m_sArtPanel->SetWndPos(0 +(mc-ac)+ iIndent, 0);
		}
		m_sArtPanel->SetVisible(true);
	}
	else
		m_sArtPanel->SetVisible(false);
}


void CUIArtefactPanel::Draw()
{
#if 0
	const float iIndent = 1.0f;
		  float x = 0.0f;
		  float y = 0.0f;
		  float iHeight;
		  float iWidth;

	Frect				rect;
	GetAbsoluteRect		(rect);
	if (m_eIconsAligment == EAligment::alLeft)
	{
		x = rect.left;
		y = rect.top;
	}
	else if (m_eIconsAligment == EAligment::alRight)
	{
		x = rect.right;
		y = rect.top;
	}

	
	float _s			= m_cell_size.x/m_cell_size.y;

	for (ITr it = m_vRects.begin(); it != m_vRects.end(); ++it)
	{
		const Frect& r = *it;		

		iHeight = m_fScale*(r.bottom - r.top);
		iWidth  = _s*m_fScale*(r.right - r.left);

		m_si.SetOriginalRect(r.left, r.top, r.width(), r.height());

		m_si.SetRect(0, 0, iWidth, iHeight);

		if (m_eIconsAligment == EAligment::alLeft)
		{
			m_si.SetPos(x, y);
			x = x + iIndent + iWidth;
		}
		else if (m_eIconsAligment == EAligment::alRight)
		{
			x = x - iWidth;
			m_si.SetPos(x, y);
			x = x - iIndent;
		}
		m_si.Render();
	}
#endif
	CUIWindow::Draw();
}
