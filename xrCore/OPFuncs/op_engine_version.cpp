#include "stdafx.h"

#include "op_engine_version.h"


#define PATCH_INFO_PRESENT

//! Keep this defines here so you don't have to rebuild cpps when you change version
#define ENGINE_DESCRIPTION "OP 2.1 Engine"
#define ENGINE_MINOR "0"
#define ENGINE_MAJOR "55j"

#ifdef DEBUG
	#define ENGINE_BUILD_TYPE "debug"
#else
	#define	ENGINE_BUILD_TYPE "release"
#endif

#define PATCH_DESCRIPTION " patch"
#define PATCH_MINOR "0"
#define PATCH_MAJOR "5" 

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
