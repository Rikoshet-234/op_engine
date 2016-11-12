#pragma once

#ifdef XRGAME_EXPORTS
#	include "ui/xrUIXmlParser.h"
#else // XRGAME_EXPORTS
#	include "xrUIXmlParser.h"
#	include "object_broker.h"
#endif // XRGAME_EXPORTS


//T_ID    - уникальный текстовый идентификатор (аттрибут id в XML файле)
//T_INDEX - уникальный числовой индекс 
//T_INIT -  класс где определена статическая InitXmlIdToIndex
//          функция инициализации file_str и tag_name

//структура хранит строковый id элемента 
//файл и позицию, где этот элемент находится
struct ITEM_DATA
{
	shared_str		id;
	int				index;
	int				pos_in_file;
	CUIXml*			_xml;
};
typedef xr_vector<ITEM_DATA>	T_VECTOR;

template<typename K, typename V, class P=stdext::hash_compare<K, std::less<K> >, typename allocator = xalloc<K> >
class xr_unordered_multimap2 : public stdext::hash_multimap<K,V,P,allocator>
{ 
public: 
	u32 size() const { return (u32)__super::size(); } 
};

class XRStringMap
{
	struct TMapValue
	{
		TMapValue(const shared_str& _key, T_VECTOR::size_type _data) : key(_key), data(_data){}
		shared_str key;
		T_VECTOR::size_type data;
	};
	typedef xr_unordered_multimap2<u32, TMapValue> TMap;

public:
	typedef TMap::const_iterator const_iterator;

public:
	void insert(const shared_str& key, T_VECTOR::size_type value);
	bool exist(const shared_str& key) const;
	const_iterator find(const shared_str& key) const;
	const_iterator begin() const;
	const_iterator end() const;

	void erase(const_iterator& i);
	void clear();

private:
	TMap m_multimap;
};

void _destroy_item_data_vector_cont(T_VECTOR* vec);

#define TEMPLATE_SPECIALIZATION template<typename T_INIT>
#define CSXML_IdToIndex CXML_IdToIndex<T_INIT>

TEMPLATE_SPECIALIZATION
class CXML_IdToIndex
{
public:

private:
	static	T_VECTOR*				m_pItemDataVector;
	static	XRStringMap*			m_pItemDataMap;

protected:
	//имена xml файлов (разделенных запятой) из которых 
	//производить загрузку элементов
	static LPCSTR					file_str;
	//имена тегов
	static LPCSTR					tag_name;
public:
									CXML_IdToIndex							();
	virtual							~CXML_IdToIndex					();

	static	void					InitInternal ();

	static const ITEM_DATA*			GetById		(const shared_str& str_id, bool no_assert = false);
	static const ITEM_DATA*			GetByIndex	(int index, bool no_assert = false);

	static const int			IdToIndex	(const shared_str& str_id, int default_index = T_INDEX(-1), bool no_assert = false)
{
		const ITEM_DATA* item = GetById(str_id, no_assert);
		return item?item->index:default_index;
	}
	static const shared_str		IndexToId	(int index, shared_str default_id = NULL, bool no_assert = false)
	{
		const ITEM_DATA* item = GetByIndex(index, no_assert);
		return item?item->id:default_id;
	}

	static const int		GetMaxIndex	()					 {return m_pItemDataVector->size()-1;}

	//удаление статичекого массива
	static void					DeleteIdToIndexData		();
};


TEMPLATE_SPECIALIZATION
typename T_VECTOR* CSXML_IdToIndex::m_pItemDataVector = NULL;
TEMPLATE_SPECIALIZATION
typename XRStringMap* CSXML_IdToIndex::m_pItemDataMap = NULL;

TEMPLATE_SPECIALIZATION
LPCSTR CSXML_IdToIndex::file_str = NULL;
TEMPLATE_SPECIALIZATION
LPCSTR CSXML_IdToIndex::tag_name = NULL;


TEMPLATE_SPECIALIZATION
CSXML_IdToIndex::CXML_IdToIndex()
{
}


TEMPLATE_SPECIALIZATION
CSXML_IdToIndex::~CXML_IdToIndex()
{
}


TEMPLATE_SPECIALIZATION
const typename ITEM_DATA* CSXML_IdToIndex::GetById (const shared_str& str_id, bool no_assert)
{
	T_INIT::InitXmlIdToIndex();

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

TEMPLATE_SPECIALIZATION
const typename ITEM_DATA* CSXML_IdToIndex::GetByIndex(int index, bool no_assert)
{
	if((size_t)index>=m_pItemDataVector->size())
	{
		R_ASSERT3(no_assert, "item by index not found in files", file_str);
		return NULL;
	}
	return &(*m_pItemDataVector)[index];
}

TEMPLATE_SPECIALIZATION
void CSXML_IdToIndex::DeleteIdToIndexData	()
{
	VERIFY(m_pItemDataVector);
	VERIFY(m_pItemDataMap);
	_destroy_item_data_vector_cont	(m_pItemDataVector);

	xr_delete(m_pItemDataVector);
	xr_delete(m_pItemDataMap);
}

#ifndef TS_ENABLE
//#define TS_ENABLE
#endif

#ifndef ETS_DECLARE
	#ifdef TS_ENABLE
		#define ETS_DECLARE(x) extern CTimerStat x
		#define ETS_BEGIN(x) x.Begin()
		#define ETS_END(x) x.End()
	#else
		#define ETS_DECLARE(x)
		#define ETS_BEGIN(x)
		#define ETS_END(x)
	#endif
#endif //ETS_DECLARE

ETS_DECLARE(g_iiForOuter);
ETS_DECLARE(g_iiForInner);
ETS_DECLARE(g_iiFIFind);

TEMPLATE_SPECIALIZATION
typename void	CSXML_IdToIndex::InitInternal ()
{
	VERIFY(!m_pItemDataVector && !m_pItemDataMap);
	T_INIT::InitXmlIdToIndex();

	m_pItemDataVector = xr_new<T_VECTOR>();
	m_pItemDataMap = xr_new<XRStringMap>();

	VERIFY(file_str);
	VERIFY(tag_name);

	string_path	xml_file;
	int			count = _GetItemCount	(file_str);
	int			index = 0;
	ETS_BEGIN(g_iiForOuter);
	for (int it=0; it<count; ++it)	
	{
		_GetItem	(file_str, it, xml_file);

		CUIXml* uiXml			= xr_new<CUIXml>();
		xr_string				xml_file_full;
		xml_file_full			= xml_file;
		xml_file_full			+= ".xml";
		bool xml_result			= uiXml->Init(CONFIG_PATH, GAME_PATH, xml_file_full.c_str());
		R_ASSERT3				(xml_result, "error while parsing XML file", xml_file_full.c_str());

		//общий список
		int items_num			= uiXml->GetNodesNum(uiXml->GetRoot(), tag_name);

		ETS_BEGIN(g_iiForInner);
		for(int i=0; i<items_num; ++i)
		{
			LPCSTR item_name	= uiXml->ReadAttrib(uiXml->GetRoot(), tag_name, i, "id", NULL);

			if(!item_name)
			{
				string256 buf;
				sprintf_s(buf, "id for item don't set, number %d in %s", i, xml_file);
				R_ASSERT2(item_name, buf);
			}
			

			//проверетить ID на уникальность
			ETS_BEGIN(g_iiFIFind);
			shared_str id(item_name);
			bool exist = m_pItemDataMap->exist(id);
			ETS_END(g_iiFIFind);
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
		ETS_END(g_iiForInner);
		if(0==items_num)
			delete_data(uiXml);
	}
	ETS_END(g_iiForOuter);
}

#undef TEMPLATE_SPECIALIZATION

