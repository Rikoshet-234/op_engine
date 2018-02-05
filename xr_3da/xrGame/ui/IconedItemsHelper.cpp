#include "stdafx.h"
#include "IconedItemsHelper.h"
#include "../string_table.h"

std::pair<ALife::EHitType,shared_str> createPair(ALife::EHitType hitType)
{
	string128 str_desc;
	sprintf_s(str_desc,"ui_inv_outfit_%s_protection",ALife::g_cafHitType2String(hitType));
	return mk_pair(hitType,str_desc);
}
xr_map<ALife::EHitType,shared_str> CreateImmunesStringMap()
{
	xr_map<ALife::EHitType,shared_str> immunes;
	immunes.insert(createPair(ALife::eHitTypeBurn));
	immunes.insert(createPair(ALife::eHitTypeShock));
	immunes.insert(createPair(ALife::eHitTypeStrike));
	immunes.insert(createPair(ALife::eHitTypeWound));
	immunes.insert(createPair(ALife::eHitTypeRadiation));
	immunes.insert(createPair(ALife::eHitTypeTelepatic));
	immunes.insert(createPair(ALife::eHitTypeChemicalBurn));
	immunes.insert(createPair(ALife::eHitTypeExplosion));
	immunes.insert(createPair(ALife::eHitTypeFireWound));
	return immunes;
}

xr_map<int,restoreParam> CreateModificatorsStringMap()
{
	xr_map<int,restoreParam> restores;
	restores.insert(mk_pair(BLEEDING_RESTORE_ID,restoreParam("bleeding_restore_speed","ui_inv_bleeding","wound_incarnation_v")));
	restores.insert(mk_pair(RADIATION_RESTORE_ID,restoreParam("radiation_restore_speed","ui_inv_radiation","radiation_v")));
	restores.insert(mk_pair(HEALTH_RESTORE_ID,restoreParam("health_restore_speed","ui_inv_health","satiety_health_v")));
	restores.insert(mk_pair(SATIETY_RESTORE_ID, restoreParam("satiety_restore_speed", "ui_inv_satiety", "satiety_v")));
	restores.insert(mk_pair(POWER_RESTORE_ID,restoreParam("power_restore_speed","ui_inv_power","satiety_power_v")));
	restores.insert(mk_pair(POWER_LOSS_ID,restoreParam("power_loss","ui_inv_power_loss","")));
	restores.insert(mk_pair(JUMP_SPEED_DELTA_ID, restoreParam("jump_speed_delta", "ui_inv_jump_speed_delta", "jump_speed")));
	return restores;
}

CUIListItemIconed* findIconedItem(std::vector<CUIListItemIconed*> &basedList,LPCSTR keyValue,bool emptyParam,xmlParams xmlData)
{
	std::vector<CUIListItemIconed*>::iterator item_it=std::find_if(basedList.begin(),basedList.end(),[&](CUIListItem* item)
	{
		CUIListItemIconed* ii=smart_cast<CUIListItemIconed*>(item);
		if (!ii)
			return false;
		return ii->GetData()==(void*)keyValue;
	});
	CUIListItemIconed *item;
	if (item_it==basedList.end())
	{
		if (emptyParam)
			return nullptr;
		CUIXml uiXml;
		uiXml.Init(CONFIG_PATH, UI_PATH, xmlData.fileName.c_str());
		item=xr_new<CUIListItemIconed>();
		CUIXmlInit::InitIconedColumns(uiXml,xmlData.path.c_str(),0,item);
		item->SetData((void*)keyValue);
		item->SetAutoDelete(false);
		basedList.push_back(item);
	}
	else 
	{
		if (emptyParam)
		{
			xr_delete(*item_it);
			basedList.erase(std::remove(basedList.begin(),basedList.end(),*item_it),basedList.end());
			return nullptr;
		}
		item=*item_it;
	}
	return item;
}

void setIconedItem(xr_map<shared_str ,shared_str> iconIDs,CUIListItemIconed* item,LPCSTR iconKey,shared_str column1Value,float column2Value,int column2Type,float column3Value,int column3Type,int addParam)
{
	xr_map<shared_str ,shared_str>::iterator icon=iconIDs.find(iconKey);
	if (icon!=iconIDs.end())
	{
		if (icon->second.size()>0)
			item->SetFieldIcon(0,icon->second.c_str());
	}
	item->SetFieldText(1,CStringTable().translate(column1Value).c_str());
	bool column2ValuePresent=false;
	if (!fsimilar(column2Value, 0.0f) && column2Type!=3)
	{
		string128 buff_column2;
		switch (column2Type)
		{
			case 0:
				sprintf_s(buff_column2, "%s%+3.0f%%", (column2Value > 0.0f) ? "%c[green]" : "%c[red]", column2Value*100.0f);
				break;
			case 1:
				{
					LPCSTR color = (column2Value < 0) ? "%c[red]" : "%c[green]";
					if ((column2Value > 0 && column2Value < 1) || (column2Value<0 && column2Value>-1))
					{
						column2Value = column2Value * 1000;
						sprintf_s(buff_column2, "%s%+3.0f%s", color, column2Value, CStringTable().translate("ui_inv_aw_gr").c_str());
					}
					else
						sprintf_s(buff_column2, "%s%+3.0f%s", color, column2Value, CStringTable().translate("ui_inv_aw_kg").c_str());
				}
				break;
			case 2:
				{
					LPCSTR color = (column2Value > 0.0f) ? "%c[green]" : "%c[red]";
					xr_string units = "%";
					xr_string mult = "";
					if (addParam == BLEEDING_RESTORE_ID || addParam == RADIATION_RESTORE_ID)
					{
						color = (column2Value > 0) ? "%c[red]" : "%c[green]";
						if (addParam == RADIATION_RESTORE_ID)
							units = CStringTable().translate("ui_inv_radiation_units").c_str();
					}
					else if (addParam== POWER_LOSS_ID)
					{
						color = (column2Value > 0) ? "%c[red]" : "%c[green]";
					}
					if (column2Value > 9999)
					{
						column2Value /= 1000;
						mult = "k";
					}
					if (addParam == JUMP_SPEED_DELTA_ID)
					{
						units = "m";
						sprintf_s(buff_column2, "%s%+.1f%s%s", color, column2Value, mult.c_str(), units.c_str());
					}
					else
						sprintf_s(buff_column2, "%s%+3.0f%s%s", color, column2Value, mult.c_str(), units.c_str());
				}
				break;
			default:NODEFAULT;
		}
		item->SetFieldText(2, buff_column2);
		column2ValuePresent = true;
	} 
	else if (column2Type==3)
	{
		string128 buff_column2;
		sprintf_s(buff_column2, "%s%3.0f%%", (column2Value > 0.0f) ? "%c[green]" : "%c[red]", column2Value*100.0f);
		item->SetFieldText(2, buff_column2);
		column2ValuePresent = true;
	}
	item->SetVisibility(2,column2ValuePresent);
	bool column3ValuePresent=false;
	if( !fsimilar(column3Value, 0.0f) && column3Type!=-1)
	{
		string128 buff_column3;
		switch(column3Type)
		{
		case 0:
			sprintf_s	(buff_column3,"%s%+3.0f%%", (column3Value>0.0f)?"%c[green]":"%c[red]", column3Value*100.0f);
			break;
		case 1:
			{
				LPCSTR color = (column3Value<0)?"%c[red]":"%c[green]";
				if ((column3Value>0 && column3Value<1) || (column3Value<0 && column3Value>-1))
				{
					column3Value=column3Value*1000;
					sprintf_s	(buff_column3,"%s%+3.0f%s", color, column3Value,CStringTable().translate("ui_inv_aw_gr").c_str());
				}
				else
					sprintf_s	(buff_column3,"%s%+3.0f%s", color, column3Value,CStringTable().translate("ui_inv_aw_kg").c_str());
			}
			break;
		case 2:
			{
				LPCSTR color=(column3Value>0.0f)?"%c[green]":"%c[red]";
				xr_string units = "%";
				xr_string mult = "";
				if (addParam == BLEEDING_RESTORE_ID || addParam == RADIATION_RESTORE_ID)
				{
					color = (column3Value > 0) ? "%c[red]" : "%c[green]";
					if (addParam == RADIATION_RESTORE_ID)
						units = CStringTable().translate("ui_inv_radiation_units").c_str();
				}
				else if (addParam == POWER_LOSS_ID)
				{
					color = (column2Value > 0) ? "%c[red]" : "%c[green]";
				}
				if (column3Value > 9999)
				{
					column3Value /= 1000;
					mult = "k";
				}
				if (addParam == JUMP_SPEED_DELTA_ID)
				{
					units = "m";
					sprintf_s(buff_column3, "%s%+.1f%s%s", color, column3Value, mult.c_str(),units.c_str());
				}
				else
					sprintf_s(buff_column3, "%s%+3.0f%s%s", color, column3Value,mult.c_str(), units.c_str());

			}
			break;
		default:NODEFAULT;
		}
		item->SetFieldText(3,buff_column3);
		column3ValuePresent=true;
	}
	item->SetVisibility(3,column3ValuePresent);
}

void addSeparator(CUIListWnd* list,shared_str textId)
{
		CUIListItem* separator=xr_new<CUIListItem>();
		separator->SetHeight(5);
		separator->SetAutoDelete(true);
		separator->SetText(CStringTable().translate(textId).c_str());
		separator->SetTextAlignment(ETextAlignment::alCenter);
		separator->m_bSeparator = true;
		list->AddItem(separator);
}

void addSeparator(CUIListWnd* list)
{
		CUIListItem* separator=xr_new<CUIListItem>();
		separator->m_bSeparator = true;
		separator->SetAutoDelete(true);
		//separator->SetHeight(1);
		list->AddItem(separator);
}