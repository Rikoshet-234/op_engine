#include "pch_script.h"
#include "string_table.h"
#include <vector>
#include <sstream>
#include <iterator>
#include <string>
#include <regex>

#include "ui/xrUIXmlParser.h"
#include "xr_level_controller.h"
#include "../xrCore/OPFuncs/utils.h"
#include "ai_space.h"
#include "script_engine.h"
#include "OPFuncs/utils.h"

STRING_TABLE_DATA* CStringTable::pData = nullptr;
BOOL CStringTable::m_bWriteErrorsToLog = FALSE;
//winsor
const std::string CStringTable::idDelimiter="#|";
const size_t CStringTable::fixedSize=4000;

CStringTable::CStringTable	()
{
	Init();
}

void CStringTable::Destroy	()
{
	xr_delete(pData);
}

bool CStringTable::IDExist(const STRING_ID& str_id) const
{
	if(pData==nullptr) return false;
	return pData->m_StringTable[str_id]!=nullptr;
}

extern ENGINE_API xr_vector<xr_token>	languages_tokens;

void CStringTable::Init		()
{
	if(nullptr != pData) return;
	m_currentLanguageTag = "string:";
	m_currentLanguageTag.append(languages_tokens[psCurrentLanguageIndex].name);

	pData				= xr_new<STRING_TABLE_DATA>();
	
	//имя языка, если не задано (NULL), то первый <text> в <string> в XML
	pData->m_sLanguage	= pSettings->r_string("string_table", "language");

	LPCSTR S			= pSettings->r_string("string_table", "files");
	if (S && S[0]) 
	{
		string128	xml_file;
		int			count = _GetItemCount	(S);
		for (int it=0; it<count; ++it)	
		{
			_GetItem	(S,it, xml_file);
			Load		(xml_file);
		}
	}
}

#include "../xrCore/OPFuncs/global_timers.h"

void CStringTable::Load	(LPCSTR xml_file)
{
	CUIXml						uiXml;
	string_path					xml_file_full;
	strconcat					(sizeof(xml_file_full),xml_file_full, xml_file, ".xml");
	string_path					_s;
	if (pData->m_sLanguage.size())
		strconcat					(sizeof(_s),_s, STRING_TABLE_PATH, "\\", *(pData->m_sLanguage) );
	else
		strconcat					(sizeof(_s),_s, STRING_TABLE_PATH, "", "");
	bool xml_result				= uiXml.Init(CONFIG_PATH, _s, xml_file_full);
	if(!xml_result)
		Debug.fatal(DEBUG_INFO,"string table xml file not found %s, for language %s", xml_file_full, *(pData->m_sLanguage));

	//общий список всех записей таблицы в файле
	int string_num = uiXml.GetNodesNum		(uiXml.GetRoot(), "string");
	for(int i=0; i<string_num; ++i)
	{
		LPCSTR string_name = uiXml.ReadAttrib(uiXml.GetRoot(), "string", i, "id", nullptr);

		//VERIFY3					(pData->m_StringTable.find(string_name) == pData->m_StringTable.end(), "duplicate string table id", string_name);
		if (!(pData->m_StringTable.find(string_name) == pData->m_StringTable.end()))
		{
			Msg("! WARNING: duplicate string table id %s. Ignoring.", string_name);
			continue;
		};

		LPCSTR string_text		= uiXml.Read(uiXml.GetRoot(), m_currentLanguageTag.c_str(), i, nullptr);
		if (!string_text)// Fallback to default
			string_text = uiXml.Read(uiXml.GetRoot(), "string:text", i, nullptr);

		if (lstrlen(string_text)>fixedSize) //winsor
		{
			//split long text into more lines	
			//Msg("Text in '%s' too long,splitted.",string_name);
			std::string string_value(string_text);
			std::vector<std::string> newIds;
			while(string_value.length()>fixedSize || string_value.length()!=0)
			{
				std::string newId(string_name);	newId+="_"+std::to_string(newIds.size());
				std::string part=string_value.substr(0,fixedSize);
				pData->m_StringTable[newId.c_str()] = part.c_str();
				string_value.erase(0,fixedSize);
				newIds.push_back(newId);
			}
			std::stringstream joinedStr;
			std::copy(newIds.begin(), newIds.end(), std::ostream_iterator<std::string>(joinedStr, idDelimiter.c_str()));
			pData->m_StringTable[string_name] = joinedStr.str().c_str();
		}
		else
		{
			if(m_bWriteErrorsToLog && string_text)
				Msg("[string table] '%s' no translation in '%s'", string_name, *(pData->m_sLanguage));
			if (!string_text)
			{
				FATAL2("string table entry does not has a text",string_name);
			}

			STRING_VALUE str_val		= ParseLine(string_text, string_name, true);

			pData->m_StringTable[string_name] = str_val;
		}
	}
}

void CStringTable::ReparseKeyBindings()
{
	if(!pData)					return;
	STRING_TABLE_MAP_IT it		= pData->m_string_key_binding.begin();
	STRING_TABLE_MAP_IT it_e	= pData->m_string_key_binding.end();

	for(;it!=it_e;++it)
	{
		pData->m_StringTable[it->first]			= ParseLine(*it->second, *it->first, false);
	}
}


STRING_VALUE CStringTable::ParseLine(LPCSTR str, LPCSTR skey, bool bFirst)
{
//	LPCSTR str = "1 $$action_left$$ 2 $$action_right$$ 3 $$action_left$$ 4";
	xr_string			res;
	int k = 0;
	const char*			b;
	#define ACTION_STR "$$ACTION_"

//.	int LEN				= (int)xr_strlen(ACTION_STR);
	#define LEN			9

	string256				buff;
	string256				srcbuff;
	bool	b_hit			= false;

	while( (b = strstr( str+k,ACTION_STR)) !=0 )
	{
		buff[0]				= 0;
		srcbuff[0]			= 0;
		res.append			(str+k, b-str-k);
		const char* e		= strstr( b+LEN,"$$" );

		int len				= (int)(e-b-LEN);

		strncpy				(srcbuff,b+LEN, len);
		srcbuff[len]		= 0;
		GetActionAllBinding	(srcbuff, buff, sizeof(buff) );
		res.append			(buff, xr_strlen(buff) );

		k					= (int)(b-str);
		k					+= len;
		k					+= LEN;
		k					+= 2;
		b_hit				= true;
	};

	if(k<(int)xr_strlen(str)){
		res.append(str+k);
	}

	if(b_hit&&bFirst) pData->m_string_key_binding[skey] = str;

	return STRING_VALUE(res.c_str());
}

STRING_VALUE CStringTable::translate(const STRING_ID& str_id, bool trim) const
{
	VERIFY(pData);

	STRING_VALUE res = pData->m_StringTable[str_id];
	if (!res)
	{
		if (m_bWriteErrorsToLog && *str_id != nullptr && xr_strlen(*str_id) > 0)
			Msg("[string table] '%s' has no entry", *str_id);

		if (str_id == nullptr)
			return str_id;
		std::string resStr(str_id.c_str());
		if (trim && (resStr.front() == '"' && resStr.back() == '"'))
			OPFuncs::trimq(resStr);
		res = resStr.c_str();
	}
	else
	{
		bool splited = false;
		std::string value(res.c_str());
		std::string unitedValue;
		size_t delimiterPos = 0;
		while ((delimiterPos = value.find(idDelimiter)) != std::string::npos)
		{
			std::string parsedId = value.substr(0, delimiterPos);
			value.erase(0, delimiterPos + idDelimiter.length());
			splited = true;
			unitedValue += translate(parsedId.c_str()).c_str();
		};
		if (splited)
		{
			//Msg("%s united from parts",str_id);
			res = unitedValue.c_str();
		}
		else
			res = pData->m_StringTable[str_id];
	}
#pragma region try to find and call script function
	static std::regex regExp("#{2}([a-zA-Z_\\.0-9]+)#{2}", std::regex_constants::icase | std::regex_constants::optimize);
	std::string spc(res.c_str());
	std::smatch singleMatch;
	while (true) //надо менять по месту, не меняя оригинал. если делать for - то только одна итерация проходит
	{
		std::sregex_iterator start = std::sregex_iterator(spc.begin(), spc.end(), regExp);
		if (start != std::sregex_iterator())//до тех пор пока есть хоть одно совпадение
		{
			auto funcName = (*start).str(1);
			if (!funcName.empty())
			{
				luabind::functor<luabind::object> textFunc;
				bool result = ai().script_engine().functor(funcName.c_str(), textFunc);
				if (!result)
				{
					Msg("! ERROR function [%s] not exist for string_id[%s]", funcName.c_str(), str_id.c_str());
					return res;//если первой же функции нет - то не будем дальше и пробовать. 
				}
				try
				{
					luabind::object funcResult = textFunc(str_id.c_str());
					if (!funcResult.is_valid())
					{
						Msg("! ERROR function [%s] did not return the expected value for string_id[%s]", funcName.c_str(), str_id.c_str());
						return res;//аналогично с результатом
					}
					LPCSTR str_res;
					switch(funcResult.type())
					{
						case LUA_TBOOLEAN:
							str_res = OPFuncs::boolToStr(luabind::object_cast<bool>(funcResult));
							break;
						case LUA_TNUMBER:
							{
								std::ostringstream ss;
								ss << luabind::object_cast<float>(funcResult);
								str_res = ss.str().c_str();
							}
							break;
						case LUA_TSTRING:
							str_res = luabind::object_cast<LPCSTR>(funcResult);
							break;
						case LUA_TNIL:
						case LUA_TTABLE:
						case LUA_TLIGHTUSERDATA:
						case LUA_TFUNCTION:
						case LUA_TUSERDATA:
						case LUA_TTHREAD:
						default: 
							Msg("! ERROR function [%s] did not return the expected value for string_id[%s]", funcName.c_str(), str_id.c_str());
							return res;//неподдерживаемые типы результатов
					}
					OPFuncs::replaceAll(spc, (*start).str(), str_res);
				}
				catch (...)
				{
					Msg("! ERROR function [%s] did not return the expected value for string_id[%s]!", funcName.c_str(), str_id.c_str());
					return res;
				}
			}
			
		}
		else
			break;
	}
#pragma todo("результат - shared_str. может приводить к дублям? проверить!!!")
	return spc.c_str();
#pragma endregion
}
