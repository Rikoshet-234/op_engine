// stdafx.cpp : source file that includes just the standard includes
// xrCore.pch will be the pre-compiled header
// stdafx.obj will contain the pre-compiled type information

#if _MSC_VER >= 1900
    //! Empty typedef
    //! jarni: bug in dbghelp header
    #pragma warning(disable:4091)
    //! 'noexcept' used with no exception handling mode specified; termination on exception is not guaranteed.Specify / EHsc
    //! jarni: Don't want to enable /EHsc, maybe it will break BugTrap or something
    #pragma warning(disable:4577)
    //! declaration of 'rLineInfo' hides previous local declaration
    //! jarni: I hope BugTrap is written well and this is not an error
    #pragma warning(disable:4456)
    //! non-member operator new or delete functions may not be declared inline
    //! jarni: VS2015 doesn't line inline new, can't do it not inlined because it doesn't see it sometimes.
    #pragma warning(disable:4595)
    //! "<hash_map> is deprecated and will be REMOVED. Please use <unordered_map>. You can define _SILENCE_STDEXT_HASH_DEPRECATION_WARNINGS to acknowledge that you have received this warning."
    //! jarni: This will be removed when unordered_map will be used instead of hash_map
    #ifndef _SILENCE_STDEXT_HASH_DEPRECATION_WARNINGS
    #define _SILENCE_STDEXT_HASH_DEPRECATION_WARNINGS
    #endif
#endif

#include "stdafx.h"

// TODO: reference any additional headers you need in STDAFX.H
// and not in this file


 