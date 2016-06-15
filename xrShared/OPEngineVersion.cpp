#include "stdafx.h"
#include "OPEngineVersion.h"

namespace OPFuncs
{
	const std::string EngineDescription="CP-2.1 Engine";
	const std::string EngineMinorVersion="0";
	const std::string EngineMajorVersion="06w";

	const bool PatchPresent=true;

	const std::string PatchDescription="patch";
	const std::string PatchMinorVersion="0";
	const std::string PatchMajorVersion="1";

	XRSHARED_EXPORT bool  IsOPEngine() 
	{
		return true;
	}

	XRSHARED_EXPORT std::string GetOPEngineVersion()
	{
		std::stringstream versionEngine;
		std::stringstream patchVersion;
		versionEngine << EngineDescription << " ver " << EngineMinorVersion << "." << EngineMajorVersion ;
		if (PatchPresent)
		{
			patchVersion << " " << PatchDescription << " ver " << PatchMinorVersion << "." << PatchMajorVersion ;
			versionEngine << patchVersion.str();
		}
		return versionEngine.str();
	}

}



