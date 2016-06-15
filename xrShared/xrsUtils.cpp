#include "stdafx.h"

#include "xrsUtils.h"

#include <algorithm>
#include <string> 

namespace xrsUtils {
	XRSHARED_EXPORT std::string tolower(const char * str)
	{
		std::string result(str);
		std::transform(result.begin(), result.end(), result.begin(), ::tolower);
		return result;
	}

	XRSHARED_EXPORT std::string toupper(const char * str)
	{
		std::string result(str);
		std::transform(result.begin(), result.end(), result.begin(), ::toupper);
		return result;
	}

}