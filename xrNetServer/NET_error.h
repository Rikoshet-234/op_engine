#ifndef NET_ERROR_H
#define NET_ERROR_H

#ifdef XR_NETERROR_EXPORTS
	#define XR_NETERROR_EXPORT __declspec(dllexport)
#else
	#define XR_NETERROR_EXPORT __declspec(dllimport)
	#pragma comment(lib,"xrNetServer")
#endif

XR_NETERROR_EXPORT void LogPacketError(LPCSTR format, ...);

#endif