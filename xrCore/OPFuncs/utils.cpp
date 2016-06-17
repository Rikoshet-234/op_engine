#include "stdafx.h"

#include "utils.h"

#include <algorithm>
#include <string> 

namespace OPFuncs {
	XRCORE_API std::string tolower(const char * str)
	{
		std::string result(str);
		std::transform(result.begin(), result.end(), result.begin(), ::tolower);
		return result;
	}

	XRCORE_API std::string toupper(const char * str)
	{
		std::string result(str);
		std::transform(result.begin(), result.end(), result.begin(), ::toupper);
		return result;
	}

}