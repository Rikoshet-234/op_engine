#ifndef __XML_STR_ID_LOADER_H__
#define __XML_STR_ID_LOADER_H__
#pragma once

class CUIXml;


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

class XRStringMap
{
	struct TMapValue
	{
		TMapValue(const shared_str& _key, T_VECTOR::size_type _data) : key(_key), data(_data){}
		shared_str key;
		T_VECTOR::size_type data;
	};
	typedef xr_hashmultimap<u32, TMapValue> TStrIdxMap;

public:
	typedef TStrIdxMap::const_iterator const_iterator;

public:
	void insert(const shared_str& key, T_VECTOR::size_type value);
	bool exist(const shared_str& key) const;
	const_iterator find(const shared_str& key) const;
	const_iterator begin() const;
	const_iterator end() const;

	void erase(const_iterator& i);
	void clear();

private:
	TStrIdxMap m_multimap;
};

class CXML_IdToIndex
{
public:
	typedef void InitFunc(LPCSTR& file_str, LPCSTR& tag_name);

private:
	T_VECTOR*				m_pItemDataVector;
	XRStringMap*			m_pItemDataMap;

protected:
	//имена xml файлов (разделенных запятой) из которых 
	//производить загрузку элементов
	LPCSTR					m_file_str;
	//имена тегов
	LPCSTR					m_tag_name;

public:
	CXML_IdToIndex();
	virtual ~CXML_IdToIndex();

	void InitInternal(InitFunc& f);

	const ITEM_DATA* GetById(const shared_str& str_id, bool no_assert = false);
	const ITEM_DATA* GetByIndex	(int index, bool no_assert = false);

	const int IdToIndex(const shared_str& str_id, int default_index = -1, bool no_assert = false);
	const shared_str IndexToId(int index, shared_str default_id = NULL, bool no_assert = false);

	const int GetMaxIndex();

	IC LPCSTR GetTagName() const { return m_tag_name; }
	//удаление статичекого массива
	void DeleteIdToIndexData();
};

template<class T>
class CXML_IdToIndexBase
{
public:
	virtual ~CXML_IdToIndexBase() {}

	IC static void InitInternal() { s_map.InitInternal(T::InitXmlIdToIndex); }
	IC static void DeleteIdToIndexData() { s_map.DeleteIdToIndexData(); }

	IC static const ITEM_DATA* GetById(const shared_str& str_id, bool no_assert = false) { return s_map.GetById(str_id, no_assert); }
	IC static const ITEM_DATA* GetByIndex	(int index, bool no_assert = false) { return s_map.GetByIndex(index, no_assert); }
	
	IC static const int IdToIndex(const shared_str& str_id, int default_index = -1, bool no_assert = false) { return s_map.IdToIndex(str_id, default_index, no_assert); }
	IC static const shared_str IndexToId(int index, shared_str default_id = NULL, bool no_assert = false) { return s_map.IndexToId(index, default_id, no_assert); }

	IC static LPCSTR GetTagName() { return s_map.GetTagName(); }

	IC static const int GetMaxIndex() { return s_map.GetMaxIndex(); }

private:
	static CXML_IdToIndex s_map;
};

template<class T>
CXML_IdToIndex CXML_IdToIndexBase<T>::s_map;

#endif //__XML_STR_ID_LOADER_H__