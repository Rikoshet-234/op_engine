#include "stdafx.h"

#include "utils.h"

namespace OPFuncs
{
	std::string GetMonsterInfoStr(CAI_Stalker monster)
	{
		std::string usName=monster.Name();
		std::string secName=monster.cNameSect().c_str();
		std::string objName=monster.Name_script();
		string2048 strFmr;
		sprintf_s(strFmr,"ObjName [%s] SectionName [%s] IngameName [%s]",objName,secName,usName);
		return strFmr;
	}
}