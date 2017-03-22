////////////////////////////////////////////////////////////////////////////
//	Module 		: object_cloner.h
//	Created 	: 13.07.2004
//  Modified 	: 13.07.2004
//	Author		: Dmitriy Iassenev
//	Description : Object cloner
////////////////////////////////////////////////////////////////////////////

#pragma once

struct CCloner {
	template <typename T>
	struct CHelper {
		template <bool a>
		IC	static void clone(const T &from, T &to)
		{
			to				= from;
		}

		template <>
		IC	static void clone<true>(const T &from, T &to)
		{
			to				= xr_new<object_type_traits::remove_pointer<T>::type>(*from);
			CCloner::clone	(*from,*to);
		}
	};

	IC	static void clone(LPCSTR from, LPCSTR &to)
	{
		to							= from;
	}

	IC	static void clone(LPSTR  from, LPSTR &to)
	{
		to							= xr_strdup(from);
	}

	IC	static void clone(const shared_str &from, shared_str &to)
	{
		to							= from;
	}

	template <typename T1, typename T2>
	IC	static void clone(const std::pair<T1,T2> &from, std::pair<T1,T2> &to)
	{
		clone(const_cast<object_type_traits::remove_const<T1>::type&>(from.first),const_cast<object_type_traits::remove_const<T1>::type&>(to.first));
		clone(from.second,to.second);
	}

	template <typename T, int size>
	IC	static void clone(const svector<T,size> &from, svector<T,size> &to)
	{
		to.resize						(from.size());
		svector<T,size>::iterator		J = to.begin();
		svector<T,size>::const_iterator	I = from.begin(); 
		svector<T,size>::const_iterator	E = from.end();
		for ( ; I != E; ++I, ++J)
			clone						(*I,*J);
	}

	template <typename T1, typename T2>
	IC	static void clone(const std::queue<T1,T2> &_from_, std::queue<T1,T2> &_to_)
	{
		std::queue<T1,T2>			from = _from_;
		std::queue<T1,T2>			to;
		
		for ( ; !from.empty(); from.pop())
			to.push				(from.front());

		while (!_to_.empty())
			_to_.pop();

		for ( ; !to.empty(); to.pop()) {
			std::queue<T1,T2>::value_type	t;
			CCloner::clone			(to.front(),t);
			_to.push				(t);
		}
	}

	template <template <typename _1, typename _2> class T1, typename T2, typename T3>
	IC	static void clone(const T1<T2,T3> &_from, T1<T2,T3> &_to, bool)
	{
		T1<T2,T3>					from = _from;
		T1<T2,T3>					to;

		for ( ; !from.empty(); from.pop())
			to.push					(from.top());

		while (!_to.empty())
			_to.pop();

		for ( ; !to.empty(); to.pop()) {
			T1<T2,T3>::value_type	t;
			CCloner::clone			(to.top(),t);
			_to.push				(t);
		}
	}

	template <template <typename _1, typename _2, typename _3> class T1, typename T2, typename T3, typename T4>
	IC	static void clone(const T1<T2,T3,T4> &_from, T1<T2,T3,T4> &_to, bool)
	{
		T1<T2,T3,T4>				from = _from;
		T1<T2,T3,T4>				to;

		for ( ; !from.empty(); from.pop())
			to.push					(from.top());

		while (!_to.empty())
			_to.pop();

		for ( ; !to.empty(); to.pop()) {
			T1<T2,T3,T4>::value_type	t;
			CCloner::clone			(to.top(),t);
			_to.push				(t);
		}
	}

	template <typename T1, typename T2>
	IC	static void clone(const xr_stack<T1,T2> &from, xr_stack<T1,T2> &to)
	{
		return					(clone(from,to,true));
	}

	template <typename T1, typename T2, typename T3>
	IC	static void clone(const std::priority_queue<T1,T2,T3> &from, std::priority_queue<T1,T2,T3> &to)
	{
		return					(clone(from,to,true));
	}

	struct CHelper3 {
		template <template <typename _1> class T1, typename T2>
		IC	static void add(T1<T2> &data, typename T1<T2>::value_type &value)
		{
			data.push_back		(value);
		}

		template <typename T1, typename T2>
		IC	static void add(T1 &data, typename T2 &value)
		{
			data.insert			(value);
		}

		template <typename T>
		IC	static void clone(const T &from, T &to)
		{
			to.clear			();
			T::const_iterator	I = from.begin();
			T::const_iterator	E = from.end();
			for ( ; I != E; ++I) {
				T::value_type	t;
				CCloner::clone	(*I,t);
				add				(to,t);
			}
		}
	};

	template <typename T>
	struct CHelper4 {
		template <bool a>
		IC	static void clone(const T &from, T &to)
		{
			CHelper<T>::clone<object_type_traits::is_pointer<T>::value>(from,to);
		}

		template <>
		IC	static void clone<true>(const T &from, T &to)
		{
			CHelper3::clone(from,to);
		}
	};

	template <typename T>
	IC	static void clone(const T &from, T &to)
	{
		CHelper4<T>::clone<object_type_traits::is_stl_container<T>::value>(from,to);
	}
};

IC	void clone(LPCSTR p0, LPSTR &p1)
{
	p1				= xr_strdup(p0);
}

template <typename T>
IC	void clone(const T &p0, T &p1)
{
	CCloner::clone	(p0,p1);
}
