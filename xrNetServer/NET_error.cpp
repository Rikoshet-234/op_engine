#include "stdafx.h"
#include "../xrLua/lua_tools.h"
#include "NET_error.h"

XR_NETERROR_EXPORT void LogPacketError(LPCSTR format, ...)
{
	va_list mark;
	string1024	buf;
	va_start	(mark, format );
	int sz		= _vsnprintf(buf, sizeof(buf)-1, format, mark ); buf[sizeof(buf)-1]=0;
	va_end		(mark);
	if (sz)		Log(buf);

	if (!g_game_lua) return;	
	LPCSTR trace = get_lua_traceback(g_game_lua, 2);
	Msg("~ %s", trace);

	LogStackTrace("problem here:");
	if (IsDebuggerPresent())
		DebugBreak ();

}