// stdafx.cpp : source file that includes just the standard includes
//	BugTrap.pch will be the pre-compiled header
//	stdafx.obj will contain the pre-compiled type information

#if _MSC_VER >= 1900
    //! Empty typedef
    //! jarni: bug in dbghelp header
    #pragma warning(disable:4091)
    //! declaration of 'rLineInfo' hides previous local declaration
    //! jarni: I hope BugTrap is written well and this is not an error
    #pragma warning(disable:4456)
    //! 'inet_addr': Use inet_pton() or InetPton() instead or define _WINSOCK_DEPRECATED_NO_WARNINGS to disable deprecated API warnings
    #ifndef _WINSOCK_DEPRECATED_NO_WARNINGS
    #define _WINSOCK_DEPRECATED_NO_WARNINGS
    #endif
    //! non-member operator new or delete functions may not be declared inline
    //! jarni: VS2015 doesn't line inline new, can't do it not inlined because it doesn't see it sometimes.
    #pragma warning(disable:4595)
    //! error C2084: function 'void *operator new(std::size_t,void *) throw()' already has a body
    //! jarni: VS2015 already has placement new implementation
    #ifndef _NEW_
    #define _NEW_
    #endif
#endif //! _MSC_VER >= 1900

#include "StdAfx.h"

// TODO: reference any additional headers you need in STDAFX.H
// and not in this file
