#ifndef op_engine_versionH
#define op_engine_versionH

#pragma once

#define PATCH_INFO_PRESENT

#define ENGINE_DESCRIPTION "OP 2.1 Engine"
#define ENGINE_MINOR "0"
#define ENGINE_MAJOR "54h"

#ifdef DEBUG
	#define ENGINE_BUILD_TYPE "debug"
#else
	#define	ENGINE_BUILD_TYPE "release"
#endif

#define PATCH_DESCRIPTION " patch"
#define PATCH_MINOR "0"
#define PATCH_MAJOR "2" 

namespace OPFuncs
{
	XRCORE_API LPCSTR GetOPEngineVersion();
}

#endif