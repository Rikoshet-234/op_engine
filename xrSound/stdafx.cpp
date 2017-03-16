// stdafx.cpp : source file that includes just the standard includes
// xrSound.pch will be the pre-compiled header
// stdafx.obj will contain the pre-compiled type information

#if _MSC_VER >= 1900
    //! Empty typedef
    //! jarni: bug in dbghelp header
    //! #pragma warning(disable:4091)
    //! 'noexcept' used with no exception handling mode specified; termination on exception is not guaranteed.Specify / EHsc
    //! jarni: Don't want to enable /EHsc, maybe it will break BugTrap or something
    #pragma warning(disable:4577)
    //! declaration of 'rLineInfo' hides previous local declaration
    //! jarni: I hope BugTrap is written well and this is not an error
    //! #pragma warning(disable:4456)
    //! non-member operator new or delete functions may not be declared inline
    //! jarni: VS2015 doesn't line inline new, can't do it not inlined because it doesn't see it sometimes.
    #pragma warning(disable:4595)
#endif

#include "stdafx.h"

// TODO: reference any additional headers you need in STDAFX.H
// and not in this file

#ifdef __BORLANDC__
#	pragma comment(lib,	"eaxB.lib"			)
#	pragma comment(lib,	"vorbisfileB.lib"	)
#	pragma comment(lib,	"xrCoreB.lib"		)
#	pragma comment(lib,	"EToolsB.lib"		)
#	pragma comment(lib,	"OpenAL32B.lib"		)
#	pragma comment(lib,	"dsoundb.lib" 		)
#else
#	pragma comment(lib,	"eax.lib"			)
#if _MSC_VER < 1900
#	pragma comment(lib,	"xrCore.lib"		)
#	pragma comment(lib,	"xrCDB.lib"			)
#endif
#	pragma comment(lib,	"dsound.lib" 		)
#endif

