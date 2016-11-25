#include "stdafx.h"

#include "UIListItemIconed.h"
#include "UIXmlInit.h"
#include "UILines.h"

CUIStatic* CUIListItemIconed::GetFieldStatic(u32 fieldIndex)
{
	if (fieldIndex>fields.size()-1) //fieldIndex by 0 started
		return nullptr;
	CUIStatic	*pStatic=fields[fieldIndex];
	return pStatic;
}

CUIListItemIconed::~CUIListItemIconed()
{
	//std::for_each(fields.begin(),fields.end(),[](CUIStatic * static) {xr})
}

void CUIListItemIconed::SetFieldText(u32 fieldIndex, LPCSTR value)
{
	if (auto pStatic=GetFieldStatic(fieldIndex))
		pStatic->SetText(value);
}

LPCSTR CUIListItemIconed::GetFieldText(u32 fieldIndex)
{
	static const char empty = 0;
	LPCSTR result=&empty;
	if (auto pStatic=GetFieldStatic(fieldIndex))
		result=pStatic->GetText();
	return result;
}

void CUIListItemIconed::SetFieldIcon(u32 fieldIndex, LPCSTR textureName)
{
	if (auto pStatic=GetFieldStatic(fieldIndex))
		pStatic->InitTexture(textureName);
}


void CUIListItemIconed::InitXml(const char* path, CUIXml& uiXml)
{

	CUIStatic	*pStatic;
	string256 buf;
	strconcat(sizeof(buf),buf, path, ":static");

	int tabsCount = uiXml.GetNodesNum(path, 0, "static");

	XML_NODE* _stored_root = uiXml.GetLocalRoot();
	uiXml.SetLocalRoot(uiXml.NavigateToNode(path,0));
	string64 sname;

	for (int i = 0; i < tabsCount; ++i)
	{
		pStatic = xr_new<CUIStatic>();
		pStatic->SetAutoDelete(true);
		CUIXmlInit::InitStatic(uiXml, "static", i, pStatic);
		pStatic->SetTextAlignment(CGameFont::alLeft);
		pStatic->m_pLines->SetColoringMode(true);
		sprintf_s(sname,"column_%d", i);
		pStatic->SetWindowName(sname);
		AttachChild(pStatic);
		fields.push_back(pStatic);
	}

	//fields[0]->SetElipsis(CUIStatic::eepEnd, 0);
	uiXml.SetLocalRoot(_stored_root);

}

void CUIListItemIconed::ClearField(u32 fieldIndex)
{
	if (auto pStatic=GetFieldStatic(fieldIndex))
		pStatic->m_pLines->Reset();
}

void CUIListItemIconed::SetVisibility(u32 fieldIndex, bool visibility)
{
	if (auto pStatic=GetFieldStatic(fieldIndex))
		pStatic->Show(visibility);
}
