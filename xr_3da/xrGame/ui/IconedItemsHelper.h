#ifndef IconedItemsHelper_h
#define IconedItemsHelper_h

#include "../ai/stalker/ai_stalker.h"
#include "UIListItemIconed.h"

	//create map to combine hit type and descriprion for him
xr_map<ALife::EHitType,shared_str> CreateImmunesStringMap();

struct restoreParam
{
	shared_str paramName;
	shared_str paramDesc;
	shared_str actorParamName;
	restoreParam(){}
	restoreParam(shared_str name,shared_str desc,shared_str actor):paramName(name),paramDesc(desc),actorParamName(actor){}
};

struct xmlParams
{
	shared_str fileName;
	shared_str path;
	xmlParams() {}
	xmlParams(shared_str name,shared_str path):fileName(name),path(path) {}
};

//create map to combine restore type,descriptions and quick_access index
#define BLEEDING_RESTORE_ID 0
#define SATIETY_RESTORE_ID 1
#define RADIATION_RESTORE_ID 2
#define HEALTH_RESTORE_ID 3
#define POWER_RESTORE_ID 4
#define POWER_LOSS_ID 5
xr_map<int,restoreParam> CreateRestoresStringMap();

CUIListItemIconed* findIconedItem(std::vector<CUIListItemIconed*> &basedList,LPCSTR keyValue,bool emptyParam,xmlParams xmlData);
void setIconedItem(xr_map<shared_str ,shared_str> iconIDs,CUIListItemIconed* item,LPCSTR iconKey,shared_str column1Value,float column2Value,int column2Type,float column3Value,int column3Type,int addParam=0);
void addSeparatorWT(CUIListWnd* list);
void addSeparator(CUIListWnd* list,shared_str textId);

#endif