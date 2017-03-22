// stdafx.h : include file for standard system include files,
//  or project specific include files that are used frequently, but
//      are changed infrequently
//
// Third generation by Oles.

#ifndef stdafxH
#define stdafxH

#pragma once

#include "../xrCore/xrCore.h"

#include "psystem.h"

#ifdef __BORLANDC__
#pragma comment( lib, "x:\\xrCoreB.lib"	)
#else
#if _MSC_VER < 1900
#pragma comment( lib, "xrCore.lib"	)
#endif
#endif

#endif //stdafxH
