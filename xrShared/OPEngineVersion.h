#ifndef OPENGINEVERSIONH
#define OPENGINEVERSIONH

#include <string>
#include <iostream>
#include <iomanip>
#include <ctime>
#include <sstream>
#include "xrShared.h"

namespace OPFuncs
{
	extern const std::string EngineDescription;
	extern const std::string EngineMinorVersion;
	extern const std::string EngineMajorVersion;

	extern const bool PatchPresent;

	extern const std::string PatchDescription;
	extern const std::string PatchMinorVersion;
	extern const std::string PatchMajorVersion;

	XRSHARED_EXPORT bool IsOPEngine();
	XRSHARED_EXPORT std::string GetOPEngineVersion();
}

#endif