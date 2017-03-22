#include "pch_script.h"
#include "script_writer.h"


using namespace luabind;

void w_bool(IWriter *self_obj,bool value)
{
	self_obj->w_u8(value?1:0);
}


void CScriptWriter::script_register(lua_State *L)
{
	module(L)
	[
		class_<IWriter>("writer")
			.def("w_seek",			&IWriter::seek			)
			.def("w_tell",			&IWriter::tell			)

			.def("w_string",		static_cast<void (IWriter::*)(const char *p)>(&IWriter::w_string))
			.def("w_stringZ",		static_cast<void (IWriter::*)(LPCSTR)>(&IWriter::w_stringZ))
			.def("w_u64",			static_cast<void (IWriter::*)(u64)>(&IWriter::w_u64))
			.def("w_u32",			static_cast<void (IWriter::*)(u32)>(&IWriter::w_u32))
			.def("w_u16",			static_cast<void (IWriter::*)(u16)>(&IWriter::w_u16))
			.def("w_u8",			static_cast<void (IWriter::*)(u8)>(&IWriter::w_u8))
			.def("w_s64",			static_cast<void (IWriter::*)(s64)>(&IWriter::w_s64))
			.def("w_s32",			static_cast<void (IWriter::*)(s32)>(&IWriter::w_s32))
			.def("w_s16",			static_cast<void (IWriter::*)(s16)>(&IWriter::w_s16))
			.def("w_s8",			static_cast<void (IWriter::*)(s8)>(&IWriter::w_s8))
			.def("w_float",			static_cast<void (IWriter::*)(float)>(&IWriter::w_float))
			.def("w_bool",			&w_bool)
			.def("w_fcolor",		static_cast<void (IWriter::*)(const Fcolor &)>(&IWriter::w_fcolor))
			.def("w_fvector4",		static_cast<void (IWriter::*)(const Fvector4 &)>(&IWriter::w_fvector4))
			.def("w_fvector3",		static_cast<void (IWriter::*)(const Fvector3 &)>(&IWriter::w_fvector3))
			.def("w_fvector2",		static_cast<void (IWriter::*)(const Fvector2 &)>(&IWriter::w_fvector2))
			
			.def("w_ivector4",		static_cast<void (IWriter::*)(const Ivector4 &)>(&IWriter::w_ivector4))
			.def("w_ivector3",		static_cast<void (IWriter::*)(const Ivector3 &)>(&IWriter::w_ivector3))
			.def("w_ivector2",		static_cast<void (IWriter::*)(const Ivector2 &)>(&IWriter::w_ivector2))

			.def("w_float_q16",		static_cast<void (IWriter::*)(float,float,float)>(&IWriter::w_float_q16))
			.def("w_float_q8",		static_cast<void (IWriter::*)(float,float,float)>(&IWriter::w_float_q8))
			.def("w_angle16",		static_cast<void (IWriter::*)(float)>(&IWriter::w_angle16))
			.def("w_angle8",		static_cast<void (IWriter::*)(float)>(&IWriter::w_angle8))
			.def("w_dir",			static_cast<void (IWriter::*)(const Fvector &)>(&IWriter::w_dir))
			.def("w_sdir",			static_cast<void (IWriter::*)(const Fvector &)>(&IWriter::w_sdir))
	];
}