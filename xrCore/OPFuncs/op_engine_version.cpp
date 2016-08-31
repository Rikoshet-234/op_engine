#include "stdafx.h"

#include "op_engine_version.h"
#include "../../build_defines.h"

namespace OPFuncs
{

	XRCORE_API std::string GetOPEngineVersion()
	{
#ifdef PATCH_INFO_PRESENT
		string512 patchVersion="";
		sprintf_s(patchVersion," %s ver %s.%s",PATCH_DESCRIPTION , PATCH_MINOR , PATCH_MAJOR);
#endif
		string1024 engineVersion="";
		sprintf_s(engineVersion,"%s ver %s.%s%s %s",ENGINE_DESCRIPTION,ENGINE_MINOR,ENGINE_MAJOR,
#ifdef PATCH_INFO_PRESENT
			patchVersion
#else
			""
#endif
			,ENGINE_BUILD_TYPE);
		return std::string(engineVersion);
	}
}