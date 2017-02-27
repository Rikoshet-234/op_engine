#include "stdafx.h"
#pragma hdrstop

#include "FTimerStat.h"

/**/
CTimerStat::CTimerStat() 
	: m_accum(0)
	, m_count(0)
	, m_min((u64)-1)
	, m_max(0) 
{
}

void CTimerStat::Begin()
{ 
	++m_count; 
	m_t.Start(); 
}

void CTimerStat::End()
{ 
	u64 elapsed = m_t.GetElapsed_ticks(); 
	m_accum += elapsed;
	if (elapsed < m_min) m_min = elapsed;
	if (elapsed > m_max) m_max = elapsed;
}

void CTimerStat::Reset()
{
	m_accum = 0;
	m_count = 0;
	m_min = (u64)-1;
	m_max = 0;
}

void CTimerStat::Print(LPCSTR name)
{
	Msg("%s: Count = %7u, Elapsed = %5I64u ms, Average = %10.3f ms, Max = %10.3f ms, Min = %10.3f ms"
		, name
		, GetCount()
		, GetElapsed_ms()
		, GetAvg()
		, GetMax()
		, GetMin());
}

/*****************************************************************************/
/*                                 CTimerStatManager                         */
/*****************************************************************************/
class CTimerStatManager
{
	typedef const char* key_type;
	typedef CTimerStat value_type;

	typedef std::set<key_type, std::less<key_type>, std::allocator<key_type>> TEnablers;
	typedef std::hash_map<key_type, CTimerStat, std::hash_compare<key_type, std::less<key_type>>, std::allocator<std::pair<const key_type&, CTimerStat>>> TMap;
	typedef std::vector<key_type> TStack;

	struct TrieNode
	{
		CTimerStat& GetOrInsert(const TStack& stack, size_t level = 0)
		{
			if (level == stack.size())
				return m_timer;
			return m_children[stack[level]].GetOrInsert(stack, level + 1);
		}

		void Print(key_type name, size_t level)
		{
			std::string s("* ");
			s.append(std::string(level, ' '));
			s.append(name);
			
			m_timer.Print(s.c_str());
			m_timer.Reset();
			for (auto i = m_children.begin(); i != m_children.end(); ++i)
				i->second.Print(i->first, level + 1);
		}

		std::hash_map<key_type, TrieNode, std::hash_compare<key_type, std::less<key_type>>, std::allocator<std::pair<const key_type&, TrieNode>>> m_children;
		CTimerStat m_timer;
	};

public:
	CTimerStatManager() {}

	void Enable(key_type enabler)
	{
		m_enablers.insert(enabler);
	}

	void Disable(key_type enabler)
	{
		m_enablers.erase(enabler);
	}

	void BeginProfile(key_type key, key_type enabler)
	{
		if (m_enablers.count(enabler))
		{
			m_stack.push_back(key);
			CTimerStat& timer = m_trieTop.GetOrInsert(m_stack, 0);
			timer.Begin();
		}
	}

	void Begin(key_type key, key_type enabler)
	{
		if (m_enablers.count(enabler))
		{
			m_independent[key].Begin();
		}
	}

	void EndProfile(key_type key, key_type enabler)
	{
		if (m_enablers.count(enabler))
		{
			if (m_stack.back() != key)
				throw std::exception("End without Begin");

			CTimerStat& timer = m_trieTop.GetOrInsert(m_stack, 0);
			timer.End();
			m_stack.pop_back();
		}
	}

	void End(key_type key, key_type enabler)
	{
		if (m_enablers.count(enabler))
		{
			m_independent[key].End();
		}
	}

	void Print()
	{
		if (!m_stack.empty())
			throw std::exception("Measurement not yet complete");
		
		for (auto i = m_trieTop.m_children.begin(); i != m_trieTop.m_children.end(); ++i)
				i->second.Print(i->first, 0);

		for (auto i = m_independent.begin(); i != m_independent.end(); ++i)
			i->second.Print(i->first);
	}

	void Print(key_type key)
	{
		m_independent[key].Print(key);
	}

private:
	TStack m_stack;
	TEnablers m_enablers;
	TrieNode m_trieTop;
	TMap m_independent;
	CTimerStat m_dummyScoped;
};

XRCORE_API CTimerStatManager* g_tsm;

XRCORE_API void TSM_Init()
{
	g_tsm = new CTimerStatManager();
}

XRCORE_API void TSM_DeInit()
{
	delete g_tsm;
}

XRCORE_API void TSM_Enable(const char* enabler)
{
	g_tsm->Enable(enabler);
}

XRCORE_API void TSM_Disable(const char* enabler)
{
	g_tsm->Disable(enabler);
}

XRCORE_API void TSM_BeginProfile(const char* name, const char* enabler)
{
	g_tsm->BeginProfile(name, enabler);
}

XRCORE_API void TSM_EndProfile(const char* name, const char* enabler)
{
	g_tsm->EndProfile(name, enabler);
}

XRCORE_API void TSM_Print()
{
	g_tsm->Print();
}

XRCORE_API void TSM_Begin(const char* name, const char* enabler)
{
	g_tsm->Begin(name, enabler);
}

XRCORE_API void TSM_End(const char* name, const char* enabler)
{
	g_tsm->End(name, enabler);
}

XRCORE_API void TSM_Print(const char* name)
{
	g_tsm->Print(name);
}
