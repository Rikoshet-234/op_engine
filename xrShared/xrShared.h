#ifndef xrSharedH
#define xrSharedH

#include <algorithm>
#include <limits>
#include <vector>
#include <stack>
#include <list>
#include <set>
#include <map>
#include <string>
#include <windows.h>

#include "xrCore.h"

#ifdef XRSHARED_EXPORTS
	#define XRSHARED_EXPORT __declspec(dllexport)
#else
	#define XRSHARED_EXPORT __declspec(dllimport)
	#pragma comment(lib,"xrShared")
#endif 

#define ERROR_MSG_PREFIX "~"
#define WARNING_MSG_PREFIX "*"


#endif