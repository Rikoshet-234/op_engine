#include "stdafx.h"

#include "op_engine_version.h"

#include "fmt/format.h"
#include "../../xrShared/resource.h"
#include "LoadStrings.h"


namespace OPFuncs
{
	XRCORE_API std::string GetOPEngineVersion()
	{
		std::string resDllFile="xrShared.dll";
		HMODULE xrSharedDllHeader = LoadLibrary(resDllFile.c_str());
		R_ASSERT2(xrSharedDllHeader,fmt::format("Can't load {0}",resDllFile).c_str());
		std::string EngineDescription=LoadString_S(IDS_VER_DESCRIPTION,xrSharedDllHeader);
		std::string EngineMinorVersion=LoadString_S(IDS_VER_MINOR,xrSharedDllHeader);
		std::string EngineMajorVersion=LoadString_S(IDS_VER_MAJOR,xrSharedDllHeader);

#ifdef DEBUG
		std::string build=LoadString_S(IDS_ENGINE_BUILD_DEBUG,xrSharedDllHeader);
#else
		std::string build=LoadString_S(IDS_ENGINE_BUILD_REL,xrSharedDllHeader);
#endif
		std::string PatchDescription=LoadString_S(IDS_PATCH_DESCRIPTION,xrSharedDllHeader);
		std::string PatchMinorVersion=LoadString_S(IDS_PATCH_MINOR,xrSharedDllHeader);
		std::string PatchMajorVersion=LoadString_S(IDS_PATCH_MAJOR,xrSharedDllHeader);
		FreeLibrary(xrSharedDllHeader);

		std::string patchVersion;
		if (PatchDescription=="patch")
		{
			patchVersion=fmt::format(" {0} ver {1}.{2}",PatchDescription , PatchMinorVersion , PatchMajorVersion);
		}
		std::string version=fmt::format("{0} ver {1}.{2}{3} {4}",EngineDescription,EngineMinorVersion,EngineMajorVersion,patchVersion,build);
		return version;
	}
}