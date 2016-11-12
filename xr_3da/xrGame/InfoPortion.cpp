#include "pch_script.h"
#include "InfoPortion.h"
#include "gameobject.h"
#include "encyclopedia_article.h"
#include "gametask.h"
#include "ai_space.h"
#include "alife_simulator.h"
#include "alife_story_registry.h"
#include "xrServer_Objects_ALife.h"
#include "script_engine.h"
#include "ui\uixmlinit.h"
#include "object_broker.h"

void INFO_DATA::load (IReader& stream) 
{
	load_data(info_id, stream); 
	load_data(receive_time, stream);
}

void INFO_DATA::save (IWriter& stream) 
{
	save_data(info_id, stream); 
	save_data(receive_time, stream);
}

void KNOWN_INFO_VECTOR::load(IReader& stream) 
{
	//! A bit hackish reading
	u32 count = stream.r_u32();
	for (u32 i = 0; i < count; ++i)
	{
		INFO_DATA temp;
		load_data(temp, stream);
		m_unordered_multimap.insert(value_type(temp.info_id._get()->dwCRC, temp));
	}
}

void KNOWN_INFO_VECTOR::save(IWriter& stream) 
{
	u32 count = m_unordered_multimap.size();
	stream.w_u32(count);
	for (auto i = m_unordered_multimap.begin(); i != m_unordered_multimap.end(); ++i)
	{
		save_data(i->second, stream);
	}
}

void KNOWN_INFO_VECTOR::insert(const value_type& vt)
{
	bool found = exist(vt.second.info_id);
	
	if (!found)
	{
		m_unordered_multimap.insert(vt);
	}
}

bool KNOWN_INFO_VECTOR::exist(const shared_str& key) const
{
	auto range = equal_range(key);
	bool found = false;
	for(auto i = range.first; i != range.second && !found; ++i)
	{
		found = i->second.info_id == key;
	}
	return found;
}

KNOWN_INFO_VECTOR::const_iterator KNOWN_INFO_VECTOR::find(const shared_str& key) const
{
	auto range = equal_range(key);
	bool found = false;
	for(auto i = range.first; i != range.second && !found; ++i)
	{
		if(i->second.info_id == key)
			return i;
	}
	return m_unordered_multimap.end();
}

//iterator begin() { return m_unordered_multimap.begin(); }
KNOWN_INFO_VECTOR::const_iterator KNOWN_INFO_VECTOR::begin() const
{
	return m_unordered_multimap.begin();
}

//iterator end() { return m_unordered_multimap.end(); }
KNOWN_INFO_VECTOR::const_iterator KNOWN_INFO_VECTOR::end() const
{
	return m_unordered_multimap.end();
}

//void erase(iterator& i)
//{
//	m_unordered_multimap.erase(i);
//}

void KNOWN_INFO_VECTOR::erase(const_iterator& i) 
{ 
	m_unordered_multimap.erase(i);
}

void KNOWN_INFO_VECTOR::clear()
{
	m_unordered_multimap.clear();
}


SInfoPortionData::SInfoPortionData ()
{
}
SInfoPortionData::~SInfoPortionData ()
{
}

CInfoPortion::CInfoPortion()
{
}

CInfoPortion::~CInfoPortion ()
{
}

void CInfoPortion::Load	(shared_str info_id)
{
	m_InfoId = info_id;
	inherited_shared::load_shared(m_InfoId, nullptr);
}


void CInfoPortion::load_shared	(LPCSTR)
{
	auto id_index=id_to_index::GetById(m_InfoId,true);
	if (id_index==nullptr)
	{
		Msg("! ERROR Infoportion [%s] not found in configs!",m_InfoId.c_str());
		FATAL("ENGINE Crush! See log for detail!");
	}
	const ITEM_DATA& item_data = *id_index;

	CUIXml*		pXML		= item_data._xml;
	pXML->SetLocalRoot		(pXML->GetRoot());

	//loading from XML
	XML_NODE* pNode			= pXML->NavigateToNode(id_to_index::tag_name, item_data.pos_in_file);
	THROW3					(pNode, "info_portion id=", *item_data.id);

	//список названий диалогов
	int dialogs_num			= pXML->GetNodesNum(pNode, "dialog");
	info_data()->m_DialogNames.clear();
	for(int i=0; i<dialogs_num; ++i)
	{
		shared_str dialog_name = pXML->Read(pNode, "dialog", i,"");
		info_data()->m_DialogNames.push_back(dialog_name);
	}

	
	//список названий порций информации, которые деактивируются,
	//после получения этой порции
	int disable_num = pXML->GetNodesNum(pNode, "disable");
	info_data()->m_DisableInfo.clear();
	for(int i=0; i<disable_num; ++i)
	{
		shared_str info_id		= pXML->Read(pNode, "disable", i,"");
		info_data()->m_DisableInfo.push_back(info_id);
	}

	//имена скриптовых функций
	info_data()->m_PhraseScript.Load(pXML, pNode);


	//индексы статей
	info_data()->m_Articles.clear();
	int articles_num	= pXML->GetNodesNum(pNode, "article");
	for(int i=0; i<articles_num; ++i)
	{
		LPCSTR article_str_id = pXML->Read(pNode, "article", i, NULL);
		THROW(article_str_id);
		info_data()->m_Articles.push_back(article_str_id);
	}

	info_data()->m_ArticlesDisable.clear();
	articles_num = pXML->GetNodesNum(pNode, "article_disable");
	for(int i=0; i<articles_num; ++i)
	{
		LPCSTR article_str_id = pXML->Read(pNode, "article_disable", i, NULL);
		THROW(article_str_id);
		info_data()->m_ArticlesDisable.push_back(article_str_id);
	}
	
	info_data()->m_GameTasks.clear();
	int task_num = pXML->GetNodesNum(pNode, "task");
	for(int i=0; i<task_num; ++i)
	{
		LPCSTR task_str_id = pXML->Read(pNode, "task", i, NULL);
		THROW(task_str_id);
		info_data()->m_GameTasks.push_back(task_str_id);
	}
}

void   CInfoPortion::InitXmlIdToIndex()
{
	if(!id_to_index::tag_name)
		id_to_index::tag_name = "info_portion";
	if(!id_to_index::file_str)
		id_to_index::file_str = pSettings->r_string("info_portions", "files");
}

void _destroy_item_data_vector_cont(T_VECTOR* vec)
{
	T_VECTOR::iterator it		= vec->begin();
	T_VECTOR::iterator it_e		= vec->end();

	xr_vector<CUIXml*>			_tmp;	
	for(;it!=it_e;++it)
	{
		xr_vector<CUIXml*>::iterator it_f = std::find(_tmp.begin(), _tmp.end(), (*it)._xml);
		if(it_f==_tmp.end())
//.		{
			_tmp.push_back	((*it)._xml);
//.			Msg("%s is unique",(*it)._xml->m_xml_file_name);
//.		}else
//.			Msg("%s already in list",(*it)._xml->m_xml_file_name);

	}
//.	Log("_tmp.size()",_tmp.size());
	delete_data	(_tmp);
}

void XRStringMap::insert(const shared_str& key, T_VECTOR::size_type value)
{
	bool found = exist(key);
	
	if (!found)
	{
		m_multimap.insert(TMap::value_type(key._get()->dwCRC, TMapValue(key, value) ));
	}
}

bool XRStringMap::exist(const shared_str& key) const
{
	auto range = m_multimap.equal_range(key._get()->dwCRC);
	bool found = false;
	for(auto i = range.first; i != range.second && !found; ++i)
	{
		found = i->second.key == key;
	}
	return found;
}

XRStringMap::const_iterator XRStringMap::find(const shared_str& key) const
{
	auto range = m_multimap.equal_range(key._get()->dwCRC);
	bool found = false;
	for(auto i = range.first; i != range.second && !found; ++i)
	{
		if(i->second.key == key)
			return i;
	}
	return m_multimap.end();
}

XRStringMap::const_iterator XRStringMap::begin() const
{
	return m_multimap.begin();
}

XRStringMap::const_iterator XRStringMap::end() const
{
	return m_multimap.end();
}

void XRStringMap::erase(const_iterator& i) 
{ 
	m_multimap.erase(i);
}

void XRStringMap::clear()
{
	m_multimap.clear();
}