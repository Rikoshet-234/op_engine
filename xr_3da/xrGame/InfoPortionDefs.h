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

class CKnownInfoContainer : public IPureSerializeObject<IReader,IWriter>
{
	typedef xr_hashmultimap<u32, INFO_DATA> TInfoMap;
public:
	typedef TInfoMap::value_type value_type;
	typedef TInfoMap::const_iterator const_iterator;

	virtual void load(IReader& stream);
	virtual void save(IWriter&);

	void insert(const INFO_DATA& data);
	bool exist(const shared_str& key) const;
	const_iterator find(const shared_str& key) const;

	const_iterator begin() const;
	const_iterator end() const;

	void erase(const_iterator& i);
	void clear();

private:
	TInfoMap m_map;
};