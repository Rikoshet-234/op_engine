﻿#include "stdafx.h"

#include "utils.h"
CTimerStat forCellCreation; 
CTimerStat forFillActor; 
CTimerStat forFillOther; 
namespace OPFuncs
{
	
	double round(double number)
	{
		return number < 0.0 ? ceil(number - 0.5) : floor(number + 0.5);
	}


	std::string GetMonsterInfoStr(CAI_Stalker monster)
	{
		std::string usName=monster.Name();
		std::string secName=monster.cNameSect().c_str();
		std::string objName=monster.Name_script();
		string2048 strFmr;
		sprintf_s(strFmr,"ObjName [%s] SectionName [%s] IngameName [%s]",objName,secName,usName);
		return strFmr;
	}

	void splitString(const std::string &s, char delim, std::vector<std::string> &elems) 
	{
		std::stringstream ss;
		ss.str(s);
		std::string item;
		while (getline(ss, item, delim)) {
			elems.push_back(item);
		}
	}


	std::vector<std::string> splitString(const std::string &s, char delim) 
	{
		std::vector<std::string> elems;
		splitString(s, delim, elems);
		return elems;
	}
}