#ifndef AFX_STDAFX_H__
#define AFX_STDAFX_H__

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#if _MSC_VER >= 1900

    //! 'noexcept' used with no exception handling mode specified; termination on exception is not guaranteed.Specify / EHsc
    //! jarni: Don't want to enable /EHsc, maybe it will break BugTrap or something
    #pragma warning(disable:4577)

#endif

#define WIN32_LEAN_AND_MEAN		// Exclude rarely-used stuff from Windows headers

#include "WarningsOff.h"
#include <windows.h>
#include <stdlib.h>
#include <tchar.h>
#include "PSAPI.h"
#include "WarningsOn.h"
#include "../xrCore.h"

namespace BlackBox {

bool isspace( int ch ); 

bool isdigit( int ch );

long atol( const char* nptr );

};

//#ifdef _EDITOR
#ifndef min
#   define min(a,b) ((a) < (b) ? (a) : (b))
#endif // _EDITOR

#endif //
