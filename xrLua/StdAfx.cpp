// stdafx.cpp : source file that includes just the standard includes
// stdafx.obj will contain the pre-compiled type information

#if _MSC_VER >= 1900
    //! Empty typedef
    //! jarni: bug in dbghelp header
    //! #pragma warning(disable:4091)
    //! 'noexcept' used with no exception handling mode specified; termination on exception is not guaranteed.Specify / EHsc
    //! jarni: Don't want to enable /EHsc, maybe it will break BugTrap or something
    //! #pragma warning(disable:4577)
    //! declaration of 'rLineInfo' hides previous local declaration
    //! jarni: I hope BugTrap is written well and this is not an error
    //! #pragma warning(disable:4456)

    //! non-member operator new or delete functions may not be declared inline
    //! jarni: VS2015 doesn't line inline new, can't do it not inlined because it doesn't see it sometimes.
    #pragma warning(disable:4595)
#endif

#include "stdafx.h"
