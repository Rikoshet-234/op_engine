#include "stdafx.h"
#include "UITableBox.h"

#include "UIXmlInit.h"
#include "UITextureMaster.h"

CUITableBox::CUITableBox() 
{
	//m_flags.set(eItemsSelectabe,FALSE);
	//m_flags.set(eFixedScrollBar,FALSE);

}

CUITableBox::~CUITableBox()
{
	xr_delete(rowStaticCache);
	std::for_each(columnsStaticCache.begin(),columnsStaticCache.end(),[](CUIStatic* temp){xr_delete(temp);});
}

void CUITableBox::InitFromXml(CUIXml& xml,LPCSTR elementName)
{
	CUIXmlInit::InitScrollView(xml,elementName,0,this);

	string256					_buff;
	strconcat(sizeof(_buff),_buff, elementName, ":row_item");
	rowStaticCache=xr_new<CUIStatic>();
	CUIXmlInit::InitStatic(xml,_buff,0,rowStaticCache);
	int index=1;
	sprintf_s(_buff,"%s:row_item:column%i",elementName,index);
	while (xml.NavigateToNode(_buff))
	{
		CUITableColumn* column=xr_new<CUITableColumn>();
		CUIXmlInit::InitStatic(xml,_buff,0,column);
		column->ShowText=xml.ReadAttribFlt(_buff,0,"text",1)?true:false;
		column->ShowIcon=xml.ReadAttribFlt(_buff,0,"icon",0)?true:false;;
		columnsStaticCache.push_back(column);
		index++;
		sprintf_s(_buff,"%s:row_item:column%i",elementName,index);
	}
	sprintf_s(_buff,"%s:icons",elementName);
	XML_NODE* iconsNode		= xml.NavigateToNode(_buff,0);

	if (iconsNode)
	{
		for (XML_NODE* node=iconsNode->FirstChild(); node; node=node->NextSibling())
		{
			if (node)
			{	
				LPCSTR id=node->Value();
				LPCSTR value=nullptr;
				XML_NODE *data=node->FirstChild();
				if (data)
				{
					TiXmlText *text			= data->ToText();
					if (text)				
						value=text->Value();
					AddIconID(id,value);
				}
			}
		}
	}
}


void CUITableBox::AddRow(xr_vector<shared_str> columnValues)
{
	if (columnValues.size()!=columnsStaticCache.size())
	{
		Msg("~ WARNING incorrect input count of values for columns. Ignore!");
		return;
	}
	CUIStatic* row=xr_new<CUIStatic>();
	Frect rect=rowStaticCache->GetWndRect();
	row->Init(rect.x1,rect.y1,rect.width(),rect.height());
	if (ref_shader shader=rowStaticCache->GetShader())
	{
		row->SetShader(shader);
		row->SetOriginalRect(rowStaticCache->GetUIStaticItem().GetOriginalRect());
	}
	for ( xr_vector<CUITableColumn*>::iterator it = columnsStaticCache.begin(); it != columnsStaticCache.end(); ++it )
	{
		size_t index = std::distance( columnsStaticCache.begin(), it );
		CUIStatic *column=xr_new<CUIStatic>();
		Frect rectl=(*it)->GetWndRect();
		column->Init(rectl.x1,rectl.y1,rectl.width(),rectl.height());
		if (ref_shader shader=(*it)->GetShader())
		{
			column->SetShader(shader);
			column->SetOriginalRect((*it)->GetUIStaticItem().GetOriginalRect());	
		}
		
		if ((*it)->ShowText)
		{
			column->SetTextColor((*it)->GetTextColor());
			column->SetFont((*it)->GetFont());
			column->SetTextAlignment((*it)->GetTextAlignment());
			column->SetText((*(columnValues.begin()+index)).c_str());
		}
		if ((*it)->ShowIcon)
		{
			shared_str id=(*(columnValues.begin()+index)).c_str();
			auto valueIt=iconIDs.find(id);
			if (valueIt==iconIDs.end())
			{
				Msg("~ WARNING incorrect column type or invalid icon id for [%s]",id.c_str());
				continue;
			}
			LPCSTR textureName=valueIt->second.c_str();
			column->InitTexture(textureName);
			column->SetTextureColor((*it)->GetTextureColor());
			column->SetTextureOffset((*it)->GetTextureOffeset().x,(*it)->GetTextureOffeset().y);
			column->SetStretchTexture((*it)->GetStretchTexture());
		}
		column->SetAutoDelete(true);
		row->AttachChild(column);
	}
	this->AddWindow(row,true);
}

void CUITableBox::AddIconID(shared_str id, shared_str value)
{
	iconIDs.insert(mk_pair(id,value));
}

