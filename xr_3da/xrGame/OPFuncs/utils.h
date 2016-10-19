#ifndef xrgUtilsH
#define xrgUtilsH

#include <string>
#include "../ai/stalker/ai_stalker.h"

namespace OPFuncs
{
	std::string GetMonsterInfoStr(CAI_Stalker);
	void splitString(const std::string &s, char delim, std::vector<std::string> &elems);
	std::vector<std::string> splitString(const std::string &s, char delim);
	double round(double number);
}

#endif