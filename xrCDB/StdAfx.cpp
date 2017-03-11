// stdafx.cpp : source file that includes just the standard includes
//	xrCDB.pch will be the pre-compiled header
//	stdafx.obj will contain the pre-compiled type information

#if _MSC_VER >= 1900
    //! 'noexcept' used with no exception handling mode specified; termination on exception is not guaranteed.Specify / EHsc
    //! jarni: Don't want to enable /EHsc, maybe it will break BugTrap or something
    #pragma warning(disable:4577)

    //! non-member operator new or delete functions may not be declared inline
    //! jarni: VS2015 doesn't line inline new, can't do it not inlined because it doesn't see it sometimes.
    #pragma warning(disable:4595)
#endif

#include "stdafx.h"
#pragma hdrstop

#ifdef __BORLANDC__
	#pragma comment(lib,"xrCoreB.lib")
#else
	#pragma comment(lib,"xrCore.lib")
#endif

#pragma comment(lib,"winmm.lib")

// TODO: reference any additional headers you need in STDAFX.H
// and not in this file
