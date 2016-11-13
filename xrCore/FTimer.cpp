#include "stdafx.h"
#pragma hdrstop

#include "FTimerStat.h"

XRCORE_API BOOL			g_bEnableStatGather	= FALSE;

CStatTimer::CStatTimer()
{
	accum	= 0;
	result	= 0.f;
	count	= 0;
}

void	CStatTimer::FrameStart	()
{
	accum	= 0;
	count	= 0;
}
void	CStatTimer::FrameEnd	()
{
	float _time			= 1000.f*float(double(accum)/double(CPU::qpc_freq)	)	;
	if (_time > result)	result	=	_time		;
	else				result	=	0.99f*result + 0.01f*_time;
}

XRCORE_API pauseMngr	g_pauseMngr;


pauseMngr::pauseMngr	():m_paused(FALSE)
{
	m_timers.reserve	(3);
}

void pauseMngr::Pause(BOOL b)
{
	if(m_paused == b)return;

	xr_vector<CTimer_paused*>::iterator it = m_timers.begin();
	for(;it!=m_timers.end();++it)
		(*it)->Pause(b);

	m_paused = b;
}

void pauseMngr::Register (CTimer_paused* t){
		m_timers.push_back(t);
}

void pauseMngr::UnRegister (CTimer_paused* t){
	xr_vector<CTimer_paused*>::iterator it = std::find(m_timers.begin(),m_timers.end(),t);
	if( it!=m_timers.end() )
		m_timers.erase(it);
}


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