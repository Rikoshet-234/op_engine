////////////////////////////////////////////////////////////////////////////
//	Module 		: script_ini_file_script.cpp
//	Created 	: 25.06.2004
//  Modified 	: 25.06.2004
//	Author		: Dmitriy Iassenev
//	Description : Script ini file class export
////////////////////////////////////////////////////////////////////////////

#include "pch_script.h"
#include "script_ini_file.h"

using namespace luabind;

CScriptIniFile *get_system_ini()
{
	return	((CScriptIniFile*)pSettings);
}

#ifdef XRGAME_EXPORTS
CScriptIniFile *get_game_ini()
{
	return	((CScriptIniFile*)pGameIni);
}
#endif // XRGAME_EXPORTS

bool r_line(CScriptIniFile *pself, LPCSTR S, int L,	xr_string &N, xr_string &V)
{
	THROW3			(pself->section_exist(S),"Cannot find section",S);
	THROW2			((int)pself->line_count(S) > L,"Invalid line number");
	
	N				= "";
	V				= "";
	
	LPCSTR			n,v;
	bool			result_ = !!pself->r_line(S,L,&n,&v);
	if (!result_)
		return		(false);

	N				= n;
	if (v)
		V			= v;
	return			(true);
}

#pragma warning(push)
#pragma warning(disable:4238)
CScriptIniFile *create_ini_file	(LPCSTR ini_string)
{
	return			(
		(CScriptIniFile*)
		xr_new<CInifile>(
			&IReader			(
				(void*)ini_string,
				xr_strlen(ini_string)
			),
			FS.get_path("$game_config$")->m_Path
		)
	);
}

CScriptIniFile *open_ini_file(LPCSTR fileName,bool modeWrite)
{
	return xr_new<CScriptIniFile>(modeWrite,fileName);
}

void close_ini_file(CScriptIniFile *file)
{
	file->save_as();
}
#pragma warning(pop)

#pragma optimize("s",on)
void CScriptIniFile::script_register(lua_State *L)
{
	module(L)
	[
		class_<CScriptIniFile>("ini_file")
			.def(					constructor<LPCSTR>())
			.def("section_exist",	&CScriptIniFile::section_exist	)
			.def("line_exist",		&CScriptIniFile::line_exist		)
			.def("r_clsid",			&CScriptIniFile::r_clsid		)
			.def("r_bool",			&CScriptIniFile::r_bool			)
			.def("r_token",			&CScriptIniFile::r_token		)
			.def("r_string_wq",		&CScriptIniFile::r_string_wb	)
			.def("line_count",		&CScriptIniFile::line_count)
			.def("r_string",		&CScriptIniFile::r_string)
			.def("r_u32",			&CScriptIniFile::r_u32)
			.def("r_s32",			&CScriptIniFile::r_s32)
			.def("r_float",			&CScriptIniFile::r_float)
			.def("r_vector",		&CScriptIniFile::r_fvector3)
			.def("r_line",			&::r_line, out_value(_4) + out_value(_5))

			.def("w_string",		static_cast<void	(CScriptIniFile::*)		(LPCSTR, LPCSTR, LPCSTR, LPCSTR )>			(&CInifile::w_string))
			.def("w_string",		static_cast<void	(CScriptIniFile::*)		(LPCSTR, LPCSTR, LPCSTR)>					(&CInifile::w_stringWC))
			.def("w_bool",			static_cast<void	(CScriptIniFile::*)		(LPCSTR, LPCSTR, bool, LPCSTR )>			(&CScriptIniFile::w_bool))
			.def("w_u32",			static_cast<void	(CScriptIniFile::*)		(LPCSTR, LPCSTR, u32, LPCSTR )>				(&CScriptIniFile::w_u32))
			.def("w_s32",			static_cast<void	(CScriptIniFile::*)		(LPCSTR, LPCSTR, s32, LPCSTR )>				(&CScriptIniFile::w_s32))
			.def("w_float",			static_cast<void	(CScriptIniFile::*)		(LPCSTR, LPCSTR, float, LPCSTR )>			(&CScriptIniFile::w_float))
			.def("w_fvector2",		static_cast<void	(CScriptIniFile::*)		(LPCSTR, LPCSTR, const Fvector2&, LPCSTR )>	(&CScriptIniFile::w_fvector2))
			.def("w_fvector3",		static_cast<void	(CScriptIniFile::*)		(LPCSTR, LPCSTR, const Fvector3&, LPCSTR )>	(&CScriptIniFile::w_fvector3))
			.def("w_fvector4",		static_cast<void	(CScriptIniFile::*)		(LPCSTR, LPCSTR, const Fvector4&, LPCSTR )>	(&CScriptIniFile::w_fvector4))
			.def("remove_line",		static_cast<void	(CScriptIniFile::*)		(LPCSTR, LPCSTR )>							(&CScriptIniFile::remove_line))
			.def("remove_section",	static_cast<void	(CScriptIniFile::*)		(LPCSTR )>									(&CScriptIniFile::remove_section)),


		def("system_ini",			&get_system_ini),
#ifdef XRGAME_EXPORTS
		def("game_ini",				&get_game_ini),
#endif // XRGAME_EXPORTS
		def("create_ini_file",		&create_ini_file,	adopt(result)),
		def("open_ini_file",		&open_ini_file,		adopt(result)),
		def("close_ini_file",		&close_ini_file)
	];
}