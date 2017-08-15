////////////////////////////////////////////////////////////////////////////
//	Module 		: script_engine.cpp
//	Created 	: 01.04.2004
//  Modified 	: 01.04.2004
//	Author		: Dmitriy Iassenev
//	Description : XRay Script Engine
////////////////////////////////////////////////////////////////////////////

#include "pch_script.h"
#include "script_engine.h"
#include "ai_space.h"
#include "object_factory.h"
#include "script_process.h"
#include "../../xrLua/lua_tools.h"
#include "../../xrCore/OPFuncs/ExpandedCmdParams.h"

#ifdef USE_DEBUGGER
#	include "script_debugger.h"
#endif

#ifndef XRSE_FACTORY_EXPORTS
#	ifdef DEBUG
#		include "ai_debug.h"
		extern Flags32 psAI_Flags;
#	endif
#endif

extern void export_classes(lua_State *L);

CScriptEngine::CScriptEngine			()
{
	m_stack_level			= 0;
	m_reload_modules		= false;
	m_last_no_file_length	= 0;
	*m_last_no_file			= 0;

#ifdef USE_DEBUGGER
	m_scriptDebugger		= nullptr;
	restartDebugger			();	
#endif
}


CScriptEngine::~CScriptEngine			()
{
	g_game_lua = nullptr;
	while (!m_script_processes.empty())
		remove_script_process(m_script_processes.begin()->first);

#ifdef DEBUG
	flush_log				();
#endif // DEBUG

#ifdef USE_DEBUGGER
	xr_delete (m_scriptDebugger);
#endif
}

void CScriptEngine::unload				()
{
	lua_settop				(lua(),m_stack_level);
	m_last_no_file_length	= 0;
	*m_last_no_file			= 0;
}

int CScriptEngine::lua_panic			(lua_State *L)
{
	print_output	(L,"PANIC",LUA_ERRRUN);
	return			(0);
}

void CScriptEngine::lua_error			(lua_State *L)
{
	print_output			(L,"",LUA_ERRRUN);

#if !XRAY_EXCEPTIONS
	LPCSTR traceback = get_lua_traceback(L, 1);
	const char *error = lua_tostring(L, -1);
	Debug.fatal(DEBUG_INFO, "LUA error: %s \n %s ", error ? error : "NULL", traceback);

#else
	throw					lua_tostring(L,-1);
#endif
}

int  CScriptEngine::lua_pcall_failed	(lua_State *L)
{
	print_output			(L,"",LUA_ERRRUN);
#if !XRAY_EXCEPTIONS
	Debug.fatal				(DEBUG_INFO,"LUA error: %s",lua_isstring(L,-1) ? lua_tostring(L,-1) : "");
#endif
	if (lua_isstring(L,-1))
		lua_pop				(L,1);
	return					(LUA_ERRRUN);
}

void lua_cast_failed					(lua_State *L, LUABIND_TYPE_INFO info)
{
	CScriptEngine::print_output	(L,"",LUA_ERRRUN);

	Debug.fatal				(DEBUG_INFO,"LUA error: cannot cast lua value to %s",info->name());
}

void CScriptEngine::setup_callbacks		()
{
#ifdef USE_DEBUGGER
	if( debugger() )
		debugger()->PrepareLuaBind	();
#endif

#ifdef USE_DEBUGGER
	if (!debugger() || !debugger()->Active() ) 
#endif
	{
#if !XRAY_EXCEPTIONS
		luabind::set_error_callback		(CScriptEngine::lua_error);
#endif
#ifndef MASTER_GOLD
		luabind::set_pcall_callback		(CScriptEngine::lua_pcall_failed);
#endif // MASTER_GOLD
	}

#if !XRAY_EXCEPTIONS
	luabind::set_cast_failed_callback	(lua_cast_failed);
#endif
	lua_atpanic							(lua(),CScriptEngine::lua_panic);
}

#include "script_thread.h"
void CScriptEngine::lua_hook_call		(lua_State *L, lua_Debug *dbg)
{
#ifdef DEBUG
	if (ai().script_engine().current_thread())
		ai().script_engine().current_thread()->script_hook(L,dbg);
	else
#endif
		ai().script_engine().m_stack_is_ready	= true;
}

int auto_load				(lua_State *L)
{
	if ((lua_gettop(L) < 2) || !lua_istable(L,1) || !lua_isstring(L,2)) {
		lua_pushnil	(L);
		return		(1);
	}

	ai().script_engine().process_file_if_exists(lua_tostring(L,2),false);
	lua_rawget		(L,1);
	return			(1);
}

void CScriptEngine::setup_auto_load		()
{
	lua_pushstring 						(lua(),"_G"); 
	lua_gettable 						(lua(),LUA_GLOBALSINDEX); 
	int value_index	= lua_gettop		(lua());  // alpet: во избежания оставления в стеке лишней метатаблицы
	luaL_newmetatable					(lua(),"XRAY_AutoLoadMetaTable");
	lua_pushstring						(lua(),"__index");
	lua_pushcfunction					(lua(), auto_load);
	lua_settable						(lua(),-3);
	// luaL_getmetatable					(lua(),"XRAY_AutoLoadMetaTable");
	lua_setmetatable					(lua(), value_index);
}

void CScriptEngine::init				()
{
	g_active_lua=nullptr;
	CScriptStorage::reinit				();
	luabind::open						(lua());
	setup_callbacks						();
	export_classes						(lua());
	setup_auto_load						();

	m_stack_is_ready					= true;

#	ifdef USE_DEBUGGER
		if( !debugger() || !debugger()->Active()  )
#	endif
			lua_sethook					(lua(),lua_hook_call,	LUA_MASKLINE|LUA_MASKCALL|LUA_MASKRET,	0);

	bool								save = m_reload_modules;
	m_reload_modules					= true;
	process_file_if_exists				("_G",false);
	m_reload_modules					= save;

	register_script_classes				();
	object_factory().register_script	();

#ifdef XRGAME_EXPORTS
	load_common_scripts					();
#endif
	m_stack_level						= lua_gettop(lua());
	g_game_lua = lua();
}

void CScriptEngine::remove_script_process	(const EScriptProcessors &process_id)
{
	CScriptProcessStorage::iterator	I = m_script_processes.find(process_id);
	if (I != m_script_processes.end()) {
		xr_delete						((*I).second);
		m_script_processes.erase		(I);
	}
}

void CScriptEngine::load_common_scripts()
{
#ifdef DBG_DISABLE_SCRIPTS
	return;
#endif
	string_path		S;
	FS.update_path	(S,"$game_config$","script.ltx");
	CInifile		*l_tpIniFile = xr_new<CInifile>(S);
	R_ASSERT		(l_tpIniFile);
	if (!l_tpIniFile->section_exist("common")) {
		xr_delete			(l_tpIniFile);
		return;
	}

	if (l_tpIniFile->line_exist("common","script")) {
		LPCSTR			caScriptString = l_tpIniFile->r_string("common","script");
		u32				n = _GetItemCount(caScriptString);
		string256		I;
		for (u32 i=0; i<n; ++i) {
			process_file(_GetItem(caScriptString,i,I));
			if (object("_G",strcat(I,"_initialize"),LUA_TFUNCTION)) {
//				lua_dostring			(lua(),strcat(I,"()"));
				luabind::functor<void>	f;
				R_ASSERT				(functor(I,f));
				f						();
			}
		}
	}

	xr_delete			(l_tpIniFile);
}

LPCSTR ExtractFileName(LPCSTR fname) //from alpet code ported, support for recursive script names
{
	LPCSTR result = fname;
	for (size_t c = 0; c < xr_strlen(fname); c++)
		if (fname[c] == '\\') result = &fname[c + 1];
	return result;
}

void CScriptEngine::CollectScriptFiles(td_SCRIPT_MAP &map, LPCSTR path)
{
	if (xr_strlen(path) == 0)
		return;
	string_path fname;
	xr_vector<char*> *folders = FS.file_list_open(path, FS_ListFolders);
	if (folders)
	{
		std::for_each(folders->begin(), folders->end(), [&](LPCSTR folder)
		{
			if (strstr(folder,".")!=nullptr)
			{
				strconcat(sizeof(fname), fname, path, folder);
				CollectScriptFiles(map, fname);
			}
		});
		FS.file_list_close(folders);
	}

	string_path buff;
	xr_vector<char*> *files = FS.file_list_open(path, FS_ListFiles);
	if (!files) 
		return;
	std::for_each(files->begin(), files->end(), [&](LPCSTR file)
	{
		strconcat(sizeof(fname), fname, path, file);
		if ((strstr(fname, ".script") || strstr(fname, ".lua")) && FS.exist(fname))
		{
			LPCSTR fstart = ExtractFileName(fname);
			strcpy_s(buff, sizeof(buff), fstart);
			_strlwr_s(buff, sizeof(buff));
			LPCSTR nspace = strtok(buff, ".");
			map.insert(mk_pair(nspace, fname));
		}
	});
	FS.file_list_close(files);
}

bool CScriptEngine::LookupScript(string_path &fname, LPCSTR base)
{
	string_path lc_base;
	Memory.mem_fill(fname, 0, sizeof(fname));

	if (0 == m_mScriptFiles.size())
	{
		FS.update_path(lc_base, "$game_scripts$", "");
		CollectScriptFiles(m_mScriptFiles, lc_base);
	}
	strcpy_s(lc_base, sizeof(lc_base), base);
	_strlwr_s(lc_base, sizeof(lc_base));
	auto it = m_mScriptFiles.find(lc_base);
	if (it != m_mScriptFiles.end())
	{
		strcpy_s(fname, sizeof(fname), *it->second);
		return true;
	}
	return false;
}

void CScriptEngine::process_file_if_exists(LPCSTR file_name, bool warn_if_not_exist)
{
	u32						string_length = xr_strlen(file_name);
	if (!warn_if_not_exist && no_file_exists(file_name, string_length))
		return;

	string_path				S, S1;
	if (m_reload_modules || (*file_name && !namespace_loaded(file_name)))
	{
		LookupScript(S, file_name);
		if (!warn_if_not_exist && !FS.exist(S)) {
#	ifndef XRSE_FACTORY_EXPORTS
#	ifdef DEBUG
			if (psAI_Flags.test(aiNilObjectAccess))
#endif
#	endif
			{
				if (OPFuncs::Dumper->isParamSet(OPFuncs::ExpandedCmdParams::KnownParams::dScriptNotExistsVariable))
				{
					print_stack();
					Msg("* trying to access variable %s, which doesn't exist, or to load script %s, which doesn't exist too", file_name, S1);
				}
				m_stack_is_ready = true;
			}
			add_no_file(file_name, string_length);
			return;
		}
		if (OPFuncs::Dumper->isParamSet(OPFuncs::ExpandedCmdParams::KnownParams::dScriptLoad))
			Msg("CScriptEngine::process_file_if_exists, loading script %s", S1);
		m_reload_modules = false;
		load_file_into_namespace(S, *file_name ? file_name : "_G");
	}
}

void CScriptEngine::process_file	(LPCSTR file_name)
{
	process_file_if_exists	(file_name,true);
}

void CScriptEngine::process_file	(LPCSTR file_name, bool reload_modules)
{
	m_reload_modules		= reload_modules;
	process_file_if_exists	(file_name,true);
	m_reload_modules		= false;
}

void CScriptEngine::register_script_classes		()
{
#ifdef DBG_DISABLE_SCRIPTS
	return;
#endif
	string_path					S;
	FS.update_path				(S,"$game_config$","script.ltx");
	CInifile					*l_tpIniFile = xr_new<CInifile>(S);
	R_ASSERT					(l_tpIniFile);

	if (!l_tpIniFile->section_exist("common")) {
		xr_delete				(l_tpIniFile);
		return;
	}

	m_class_registrators		= READ_IF_EXISTS(l_tpIniFile,r_string,"common","class_registrators","");
	xr_delete					(l_tpIniFile);

	u32							n = _GetItemCount(*m_class_registrators);
	string256					I;
	for (u32 i=0; i<n; ++i) {
		_GetItem				(*m_class_registrators,i,I);
		luabind::functor<void>	result;
		if (!functor(I,result)) {
			script_log			(eLuaMessageTypeError,"Cannot load class registrator %s!",I);
			continue;
		}
		result					(const_cast<CObjectFactory*>(&object_factory()));
	}
}

bool CScriptEngine::function_object(LPCSTR function_to_call, luabind::object &object, int type)
{
	if (!xr_strlen(function_to_call))
		return				(false);

	string256				name_space, function;

	parse_script_namespace	(function_to_call,name_space,function);
	if (xr_strcmp(name_space,"_G"))
		process_file		(name_space);

	if (!this->object(name_space,function,type))
		return				(false);

	luabind::object			lua_namespace	= this->name_space(name_space);
	object					= lua_namespace[function];
	return					(true);
}

#ifdef USE_DEBUGGER
void CScriptEngine::stopDebugger				()
{
	if (debugger()){
		xr_delete	(m_scriptDebugger);
		Msg			("Script debugger succesfully stoped.");
	}
	else
		Msg			("Script debugger not present.");
}

void CScriptEngine::restartDebugger				()
{
	if(debugger())
		stopDebugger();

	m_scriptDebugger = xr_new<CScriptDebugger>();
	debugger()->PrepareLuaBind();
	Msg				("Script debugger succesfully restarted.");
}
#endif

bool CScriptEngine::no_file_exists	(LPCSTR file_name, u32 string_length)
{
	if (m_last_no_file_length != string_length)
		return				(false);

	return					(!memcmp(m_last_no_file,file_name,string_length*sizeof(char)));
}

void CScriptEngine::add_no_file		(LPCSTR file_name, u32 string_length)
{
	m_last_no_file_length	= string_length;
	CopyMemory				(m_last_no_file,file_name,(string_length+1)*sizeof(char));
}

void CScriptEngine::collect_all_garbage	()
{
	lua_gc					(lua(),LUA_GCCOLLECT,0);
	lua_gc					(lua(),LUA_GCCOLLECT,0);
}

DLL_API void log_script_error(LPCSTR format, ...)
{
	string1024 line_buf;
	va_list mark;	
	va_start(mark, format);
	//int sz = _vsnprintf(line_buf, sizeof(line_buf)-1, format, mark); 	
	line_buf[sizeof(line_buf) - 1] = 0;
	va_end(mark);

	ai().script_engine().script_log(ScriptStorage::ELuaMessageType::eLuaMessageTypeError, line_buf);
}