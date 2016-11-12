#pragma once

#include "alife_space.h"
#include "object_interfaces.h"
#include <hash_map>

struct INFO_DATA : public IPureSerializeObject<IReader,IWriter>
{
	INFO_DATA			():info_id(NULL),receive_time(0)			{};
	INFO_DATA			(const shared_str& id, ALife::_TIME_ID time):info_id(id),receive_time(time){};

	virtual void		load			(IReader& stream);
	virtual void		save			(IWriter&);

	shared_str			info_id;
	//время получения нужно порции информации
	ALife::_TIME_ID		receive_time;
};

DEFINE_VECTOR		(INFO_DATA, KNOWN_INFO_VECTOR_1, KNOWN_INFO_VECTOR_1_IT);
template<typename K, typename V, class P=stdext::hash_compare<K, std::less<K> >, typename allocator = xalloc<K> >
class xr_unordered_multimap : public stdext::hash_multimap<K,V,P,allocator>
{ 
public: 
	u32 size() const { return (u32)__super::size(); } 
};

typedef xr_unordered_multimap<u32, INFO_DATA> KNOWN_INFO_VECTOR_2;
typedef KNOWN_INFO_VECTOR_2::iterator KNOWN_INFO_VECTOR_2_IT;
typedef KNOWN_INFO_VECTOR_2::const_iterator KNOWN_INFO_VECTOR_2_CIT;

class KNOWN_INFO_VECTOR : public IPureSerializeObject<IReader,IWriter>
{
	class CFindByIDPred
	{
	public:
		CFindByIDPred(const shared_str& element_to_find) {element = element_to_find;}
		bool operator () (const INFO_DATA& data) const {return data.info_id == element;}
	private:
		shared_str element;
	};

public:
	typedef KNOWN_INFO_VECTOR_2::value_type value_type;
	typedef KNOWN_INFO_VECTOR_2_IT iterator;
	typedef KNOWN_INFO_VECTOR_2::const_iterator const_iterator;
	typedef std::pair<KNOWN_INFO_VECTOR_2_IT, KNOWN_INFO_VECTOR_2_IT> range_type;
	typedef std::pair<KNOWN_INFO_VECTOR_2_CIT, KNOWN_INFO_VECTOR_2_CIT> const_range_type;

	virtual void		load			(IReader& stream);
	virtual void		save			(IWriter&);

	void insert(const value_type& vt);
	bool exist(const shared_str& key) const;
	const_iterator find(const shared_str& key) const;

	//iterator begin() { return m_unordered_multimap.begin(); }
	const_iterator begin() const;
	//iterator end() { return m_unordered_multimap.end(); }
	const_iterator end() const;
	//void erase(iterator& i)
	//{
	//	m_unordered_multimap.erase(i);
	//}

	void erase(const_iterator& i);
	void clear();

private:
	range_type equal_range(const shared_str& str)
	{
		return m_unordered_multimap.equal_range(str._get()->dwCRC);
	}

	const_range_type equal_range(const shared_str& str) const
	{
		return m_unordered_multimap.equal_range(str._get()->dwCRC);
	}

private:
	KNOWN_INFO_VECTOR_1 m_vector;
	KNOWN_INFO_VECTOR_2 m_unordered_multimap;
};

typedef KNOWN_INFO_VECTOR::iterator KNOWN_INFO_VECTOR_IT;
typedef KNOWN_INFO_VECTOR::const_iterator KNOWN_INFO_VECTOR_CIT;