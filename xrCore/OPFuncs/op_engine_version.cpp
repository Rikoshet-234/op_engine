#include "stdafx.h"

#include "op_engine_version.h"
#include "../../build_defines.h"

namespace OPFuncs
{

	XRCORE_API LPCSTR GetOPEngineVersion()
	{
		static string1024 engineVersion="";
		if (engineVersion[0] == 0)
		{
#ifdef PATCH_INFO_PRESENT
			sprintf_s(engineVersion,"%s ver %s.%s %s ver %s.%s %s"
				,ENGINE_DESCRIPTION, ENGINE_MINOR, ENGINE_MAJOR
				,PATCH_DESCRIPTION, PATCH_MINOR, PATCH_MAJOR
				,ENGINE_BUILD_TYPE);
#else
			sprintf_s(engineVersion,"%s ver %s.%s %s",ENGINE_DESCRIPTION,ENGINE_MINOR,ENGINE_MAJOR,ENGINE_BUILD_TYPE);
#endif
		}
		return engineVersion;
	}
}