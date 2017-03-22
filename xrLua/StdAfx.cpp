// stdafx.cpp : source file that includes just the standard includes
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

//! jarni: This file is added to every project to replace global new correctly.
//! See http://en.cppreference.com/w/cpp/memory/new/operator_new
//! Excerpts from article above:
//!
//! Versions (1-8) are replaceable: a user-provided non-member function with the same signature defined 
//! anywhere in the program, in any source file, replaces the default version. Its declaration does not need to be visible.
//! NOTE!!!: The behavior is undefined if more than one replacement is provided in the program for any of the replaceable 
//! allocation function, or IF A REPLACEMENT IS DEFINED WITH THE INLINE SPECIFIER.
//!
//! The standard library implementations of the nothrow versions (5-8) directly calls the corresponding throwing versions (1-4). 
//! The standard library implementation of the throwing array versions (2,4) directly calls the corresponding single-object version (1,3).
//! Thus, replacing the throwing single object allocation functions is sufficient to handle all allocations.
#if _MSC_VER >= 1900
void*   operator new(size_t size)
{
#ifdef DEBUG_MEMORY_NAME
    return Memory.mem_alloc(size ? size : 1, "C++ NEW");
#else
    return Memory.mem_alloc(size ? size : 1);
#endif
}

void    operator delete(void *p)
{
    xr_free(p);
}

void*	operator new[](size_t size)
{
#ifdef DEBUG_MEMORY_NAME
    return Memory.mem_alloc(size ? size : 1, "C++ NEW");
#else
    return Memory.mem_alloc(size ? size : 1);
#endif
}

void	operator delete[](void* p) {
    xr_free(p);
}
#endif