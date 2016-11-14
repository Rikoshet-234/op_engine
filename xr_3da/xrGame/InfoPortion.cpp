#include "pch_script.h"
#include "InfoPortion.h"
#include "ui/xrUIXmlParser.h"
//#include "gameobject.h"
//#include "encyclopedia_article.h"
//#include "gametask.h"
//#include "ai_space.h"
//#include "alife_simulator.h"
//#include "alife_story_registry.h"
//#include "xrServer_Objects_ALife.h"
//#include "script_engine.h"
//#include "ui\uixmlinit.h"
//#include "object_broker.h"

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

void CKnownInfoContainer::load(IReader& stream) 
{
	//! A bit hackish reading
	u32 count = stream.r_u32();
	for (u32 i = 0; i < count; ++i)
	{
		INFO_DATA temp;
		load_data(temp, stream);
		m_map.insert(value_type(temp.info_id._get()->dwCRC, temp));
	}
}

void CKnownInfoContainer::save(IWriter& stream) 
{
	u32 count = m_map.size();
	stream.w_u32(count);
	for (auto i = m_map.begin(); i != m_map.end(); ++i)
	{
		save_data(i->second, stream);
	}
}

void CKnownInfoContainer::insert(const INFO_DATA& data)
{
	if (!exist(data.info_id))
	{
		m_map.insert(TInfoMap::value_type(data.info_id._get()->dwCRC, data));
	}
}

bool CKnownInfoContainer::exist(const shared_str& key) const
{
	auto range = m_map.equal_range(key._get()->dwCRC);
	bool found = false;
	for(auto i = range.first; i != range.second && !found; ++i)
	{
		found = i->second.info_id == key;
	}
	return found;
}

CKnownInfoContainer::const_iterator CKnownInfoContainer::find(const shared_str& key) const
{
	auto range = m_map.equal_range(key._get()->dwCRC);
	bool found = false;
	for(auto i = range.first; i != range.second && !found; ++i)
	{
		if(i->second.info_id == key)
			return i;
	}
	return m_map.end();
}
CKnownInfoContainer::const_iterator CKnownInfoContainer::begin() const
{
	return m_map.begin();
}

CKnownInfoContainer::const_iterator CKnownInfoContainer::end() const
{
	return m_map.end();
}

void CKnownInfoContainer::erase(CKnownInfoContainer::const_iterator& i) 
{ 
	m_map.erase(i);
}

void CKnownInfoContainer::clear()
{
	m_map.clear();
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


void CInfoPortion::load_shared(LPCSTR)
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
	XML_NODE* pNode			= pXML->NavigateToNode(id_to_index::GetTagName(), item_data.pos_in_file);
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

void   CInfoPortion::InitXmlIdToIndex(LPCSTR& file_str, LPCSTR& tag_name)
{
	if(!tag_name)
		tag_name = "info_portion";
	if(!file_str)
		file_str = pSettings->r_string("info_portions", "files");
}