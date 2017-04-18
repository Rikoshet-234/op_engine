#include "stdafx.h"

#include "utils.h"

#include <algorithm> 
#include <functional> 
#include <cctype>
#include <locale>
#include  <string>

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

	XRCORE_API inline void ltrimq(std::string &s)
	{
		if ( s.front() == '"') 
		{
			s.erase( 0, 1 ); 
		}
	}

	XRCORE_API inline void rtrimq(std::string &s)
	{
		if ( s.back() == '"') 
		{
			s.erase( s.size() - 1 ); 
		}
	}

	XRCORE_API inline void trimq(std::string &s)
	{
		ltrimq(s);
		rtrimq(s);
	}

	XRCORE_API inline void ltrim(std::string &s) 
	{
		s.erase(s.begin(), std::find_if(s.begin(), s.end(),
			std::not1(std::ptr_fun<int, int>(std::isspace))));
	}


	XRCORE_API inline void rtrim(std::string &s) {
	s.erase(std::find_if(s.rbegin(), s.rend(),
			std::not1(std::ptr_fun<int, int>(std::isspace))).base(), s.end());
}


	XRCORE_API inline void trim(std::string &s) {
		ltrim(s);
		rtrim(s);
	}

	template<class T>
	XRCORE_API STR2INT_ERROR str2int (T &i, char const *s)
	{
		char *end;
		T  l;
		errno = 0;
		l = static_cast<T>(strtol(s, &end,0));
		if ((errno == ERANGE && l == LONG_MAX) || l > INT_MAX) {
			return STR2INT_ERROR::OVERRANGE;
		}
		if ((errno == ERANGE && l == LONG_MIN) || l < INT_MIN) {
			return STR2INT_ERROR::UNDERRANGE;
		}
		if (*s == '\0' || *end != '\0') {
			return STR2INT_ERROR::INCONVERTIBLE;
		}
		i = l;
		return STR2INT_ERROR::SUCCESS;
	}
}
