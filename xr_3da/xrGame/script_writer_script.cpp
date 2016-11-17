#include "pch_script.h"
#include "script_writer.h"


using namespace luabind;

void CScriptWriter::script_register(lua_State *L)
{
	module(L)
	[
		class_<IWriter>("writer")
			.def("w_seek",			&IWriter::seek			)
			.def("w_tell",			&IWriter::tell			)
			.def("w_s16",			static_cast<void (IWriter::*)(s16)>(&IWriter::w_s16))
			.def("w_stringZ",		static_cast<void (IWriter::*)(LPCSTR)>(&IWriter::w_stringZ))

	];
}