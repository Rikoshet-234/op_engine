#include "StdAfx.h"
//#include <queue>

#include "../xrCore/_stl_extensions.h"
#include "xml_str_id_loader.h"
#include "object_interfaces.h"
#include "object_destroyer.h"

#ifdef XRGAME_EXPORTS
#	include "ui/xrUIXmlParser.h"
#else // XRGAME_EXPORTS
#	include "xrUIXmlParser.h"
#	include "object_broker.h"
#endif // XRGAME_EXPORTS

//#define TS_ENABLE
#include "../xrCore/FTimerStat.h"
#undef TS_ENABLE

void _destroy_item_data_vector_cont(T_VECTOR* vec)
{
	auto it = vec->begin();
	auto it_e = vec->end();

	xr_vector<CUIXml*> _tmp;
	for(;it!=it_e;++it)
	{
		xr_vector<CUIXml*>::iterator it_f = std::find(_tmp.begin(), _tmp.end(), (*it)._xml);
		if(it_f==_tmp.end())
			_tmp.push_back	((*it)._xml);
	}

	delete_data	(_tmp);
}

/******************************************************************************/
/*                                 XRStringMap                                */
/******************************************************************************/
void XRStringMap::insert(const shared_str& key, T_VECTOR::size_type value)
{
	bool found = exist(key);
	
	if (!found)
	{
		m_multimap.insert(TStrIdxMap::value_type(key._get()->dwCRC, TMapValue(key, value) ));
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

/******************************************************************************/
/*                                 CXML_IdToIndex                             */
/******************************************************************************/
CXML_IdToIndex::CXML_IdToIndex()
	: m_pItemDataMap(NULL)
	, m_pItemDataVector(NULL)
	, m_file_str(NULL)
	, m_tag_name(NULL)
{
}

CXML_IdToIndex::~CXML_IdToIndex()
{
	if (m_pItemDataMap)
		xr_delete(m_pItemDataMap);

	if (m_pItemDataVector)
		xr_delete(m_pItemDataVector);
}

const int CXML_IdToIndex::IdToIndex(const shared_str& str_id, int default_index /*= -1*/, bool no_assert /*= false*/)
{
	const ITEM_DATA* item = GetById(str_id, no_assert);
	return item?item->index:default_index;
}

const shared_str CXML_IdToIndex::IndexToId(int index, shared_str default_id /*= NULL*/, bool no_assert /*= false*/)
{
	const ITEM_DATA* item = GetByIndex(index, no_assert);
	return item?item->id:default_id;
}

const int CXML_IdToIndex::GetMaxIndex()
{
	return m_pItemDataVector->size()-1;
}

const ITEM_DATA* CXML_IdToIndex::GetById (const shared_str& str_id, bool no_assert)
{
	auto map_i = m_pItemDataMap->find(str_id);

	if(m_pItemDataMap->end() == map_i)
	{
		int i=0;
		for(T_VECTOR::iterator it = m_pItemDataVector->begin();	m_pItemDataVector->end() != it; it++,i++)
			Msg("[%d]=[%s]",i,*(*it).id );

		R_ASSERT3(no_assert, "item not found, id", *str_id);
		return NULL;
	}
	
	return &(*m_pItemDataVector)[map_i->second.data];
}

const ITEM_DATA* CXML_IdToIndex::GetByIndex(int index, bool no_assert)
{
	if((size_t)index>=m_pItemDataVector->size())
	{
		R_ASSERT3(no_assert, "item by index not found in files", m_file_str);
		return NULL;
	}
	return &(*m_pItemDataVector)[index];
}

void CXML_IdToIndex::DeleteIdToIndexData()
{
	VERIFY(m_pItemDataVector);
	VERIFY(m_pItemDataMap);
	_destroy_item_data_vector_cont(m_pItemDataVector);

	xr_delete(m_pItemDataVector);
	xr_delete(m_pItemDataMap);
}

TSE_DECLARE(g_iiForOuter);
TSE_DECLARE(g_iiForInner);
TSE_DECLARE(g_iiFIFind);

void CXML_IdToIndex::InitInternal(CXML_IdToIndex::InitFunc& f)
{
	VERIFY(!m_pItemDataVector && !m_pItemDataMap);
	f(m_file_str, m_tag_name);

	VERIFY(file_str);
	VERIFY(tag_name);

	m_pItemDataVector = xr_new<T_VECTOR>();
	m_pItemDataMap = xr_new<XRStringMap>();

	string_path	xml_file;
	int			count = _GetItemCount(m_file_str);
	int			index = 0;
	TS_BEGIN(g_iiForOuter);
	for (int it=0; it<count; ++it)	
	{
		_GetItem(m_file_str, it, xml_file);

		CUIXml* uiXml			= xr_new<CUIXml>();
		xr_string				xml_file_full;
		xml_file_full			= xml_file;
		xml_file_full			+= ".xml";
		bool xml_result			= uiXml->Init(CONFIG_PATH, GAME_PATH, xml_file_full.c_str());
		R_ASSERT3				(xml_result, "error while parsing XML file", xml_file_full.c_str());

		//общий список
		int items_num			= uiXml->GetNodesNum(uiXml->GetRoot(), m_tag_name);

		TS_BEGIN(g_iiForInner);
		for(int i=0; i<items_num; ++i)
		{
			LPCSTR item_name	= uiXml->ReadAttrib(uiXml->GetRoot(), m_tag_name, i, "id", NULL);

			if(!item_name)
			{
				string256 buf;
				sprintf_s(buf, "id for item don't set, number %d in %s", i, xml_file);
				R_ASSERT2(item_name, buf);
			}
			

			//проверетить ID на уникальность
			TS_BEGIN(g_iiFIFind);
			shared_str id(item_name);
			bool exist = m_pItemDataMap->exist(id);
			TS_END(g_iiFIFind);
			R_ASSERT3(!exist, "duplicate item id", item_name);

			ITEM_DATA			data;
			data.id				= id;
			data.index			= index;
			data.pos_in_file	= i;
			data._xml			= uiXml;
			m_pItemDataMap->insert(id, m_pItemDataVector->size());
			m_pItemDataVector->push_back(data);

			index++; 
		}
		TS_END(g_iiForInner);
		if(0==items_num)
			delete_data(uiXml);
	}
	TS_END(g_iiForOuter);
}