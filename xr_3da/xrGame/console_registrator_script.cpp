#include "pch_script.h"
#include "console_registrator.h"
#include "../xr_ioconsole.h"
#include "../xr_ioc_cmd.h"
#include "ai_space.h"
#include "script_engine.h"
#include <luabind/luabind.hpp>
#include <luabind/object.hpp>

using namespace luabind;

CConsole*	console()
{
	return Console;
}

int get_console_integer(CConsole* c, LPCSTR cmd)
{
	int val=0,min=0,max=0;
	c->GetInteger ( cmd, val, min, max);
	return val;
}

float get_console_float(CConsole* c, LPCSTR cmd)
{
	float val=0,min=0,max=0;
	c->GetFloat ( cmd, val, min, max);
	return val;
}

bool get_console_bool(CConsole* c, LPCSTR cmd)
{
	BOOL val;
	val = c->GetBool (cmd, val);
	return !!val;
}

void set_user_param_value(CConsole* c,LPCSTR cmd,LPCSTR value)
{
	if (c->isUserDefinedParam(cmd))
	{
		CConsole::vecCMD_IT I = c->Commands.find(cmd);
		if (I!=c->Commands.end()) 
		{
			CCC_UserParam* uc=smart_cast<CCC_UserParam*>(I->second);
			if (uc)
				uc->SetValue(value);
			else
				Msg("! ERROR User-defined param [%s] not is class CCC_UserParam! Contact to developer quickly!",cmd);
		}
	}
	else
		Msg("~ WARNING User-defined param [%s] not found!",cmd);
}

luabind::object get_user_param_value(CConsole* c,LPCSTR cmd)
{
	
	if (c->isUserDefinedParam(cmd))
	{
		CConsole::vecCMD_IT I = c->Commands.find(cmd);
		if (I!=c->Commands.end()) 
		{
			CCC_UserParam* uc=smart_cast<CCC_UserParam*>(I->second);
			if (uc)
				return luabind::object(ai().script_engine().lua(),uc->GetValue().c_str());
			else
				Msg("! ERROR User-defined param [%s] not is class CCC_UserParam! Contact to developer quickly!",cmd);
		}
	}
	else
		Msg("~ WARNING User-defined param [%s] not found!",cmd);
	return luabind::object();
}

#pragma optimize("s",on)
void console_registrator::script_register(lua_State *L)
{
	module(L)
	[
		def("get_console",					&console),
		class_<CConsole>("CConsole")
		.def("execute",						&CConsole::Execute)
		.def("execute_script",				&CConsole::ExecuteScript)
		.def("show",						&CConsole::Show)
		.def("hide",						&CConsole::Hide)
//		.def("save",						&CConsole::Save)
		.def("get_user_param_value"			,get_user_param_value)
		.def("set_user_param_value"			,set_user_param_value)
		.def("get_string",					&CConsole::GetString)
		.def("get_integer",					&get_console_integer)
		.def("get_bool",					&get_console_bool)
		.def("get_float",					&get_console_float)
		.def("get_token",					&CConsole::GetToken)
//		.def("",				&CConsole::)

	];
}