#ifndef xrSharedH
#define xrSharedH

extern "C" {
	#include <lua.h>
	#include <luajit.h>
	#include <lcoco.h>
};

#include <algorithm>
#include <limits>
#include <vector>
#include <stack>
#include <list>
#include <set>
#include <map>
#include <string>
#include <windows.h>
#include "../xrCore/xrCore.h"

#ifdef XRSHARED_EXPORTS
	#define XRSHARED_EXPORT __declspec(dllexport)
#else
	#define XRSHARED_EXPORT __declspec(dllimport)
	#pragma comment(lib,"xrShared")
#endif 

#define ERROR_MSG_PREFIX "~"
#define WARNING_MSG_PREFIX "*"

template <typename TKey,typename TValue>
class createMap
{
private:
	std::map<TKey,TValue> _map;
public:
	createMap(const TKey& key, const TValue& val)
	{
		_map[key] = val;
	}

	createMap<TKey, TValue>& operator()(const TKey& key, const TValue& val)
	{
		_map[key] = val;
		return *this;
	}

	operator std::map<TKey, TValue>()
	{
		return _map;
	}
};

#endif