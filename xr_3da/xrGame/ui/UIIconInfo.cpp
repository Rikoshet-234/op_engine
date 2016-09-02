#include "stdafx.h"
#include "UIIconInfo.h"
#include "UIInventoryUtilities.h"

void UIIconInfo::Load(const shared_str itemSection,bool raise)
{
	int x=0,y=0,w=0,h=0;
	if (pSettings->line_exist(itemSection,"inv_grid_icon"))
	{
		std::string strCoords	= pSettings->r_string(itemSection,"inv_grid_icon");
		int itemsParsed=std::sscanf(strCoords.c_str(),"%i,%i,%i,%i",&x,&y,&w,&h);
		if (itemsParsed!=4 && raise)
		{
			Debug.fatal(DEBUG_INFO,"Invalid format for inv_grid_icon",itemSection.c_str());
		}
	}
	else
	{
		x=pSettings->r_u32(itemSection, "inv_grid_x");
		y=pSettings->r_u32(itemSection, "inv_grid_y");
		w=pSettings->r_u32(itemSection, "inv_grid_width");
		h=pSettings->r_u32(itemSection, "inv_grid_height");
	}
	setCoords(x,y,w,h);
}

Frect& UIIconInfo::getOriginalRect() const
{
	static Frect rect;
	rect.x1 = INV_GRID_WIDTH  * float(getX());
	rect.y1 = INV_GRID_HEIGHT * float(getY());
	rect.x2 = rect.x1 + INV_GRID_WIDTH  * getWidth();
	rect.y2 = rect.y1 + INV_GRID_HEIGHT * getHeight();
	return rect;

}
