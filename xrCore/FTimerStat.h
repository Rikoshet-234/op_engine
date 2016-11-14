#ifndef FTimerStatH
#define FTimerStatH
#pragma once

#include "FTimer.h"

class XRCORE_API CTimerStat
{
public:
	CTimer m_t;
	u64 m_accum;
	u32 m_count;
	u64 m_min;
	u64 m_max;

public:
	CTimerStat();

	void Begin();
	void End();

	IC  u64 GetElapsed_ms() const { return (m_accum * 1000) / CPU::qpc_freq; }
	IC  u32 GetCount() const { return m_count; }
	IC  double GetMin() const { return (double)(m_min * 1000) / CPU::qpc_freq; }
	IC  double GetMax() const { return (double)(m_max * 1000) / CPU::qpc_freq; }
	IC  double GetAvg() const { return ((double)(m_accum * 1000) / CPU::qpc_freq) / (double)(m_count ? m_count : 1); }
	
	void Reset();
	void Print(LPCSTR name);
};

class XRCORE_API CTimerStatScoped
{
public:
	CTimerStatScoped(CTimerStat& rTimer) : m_rTimer(rTimer) { m_rTimer.Begin(); }
	~CTimerStatScoped() { m_rTimer.End(); }
private:
	CTimerStatScoped(const CTimerStatScoped& rTimer);
	CTimerStatScoped& operator=(const CTimerStatScoped& rTimer);
	CTimerStat& m_rTimer;
};

#ifdef TS_ENABLE
	#define TS_DECLARE(x) CTimerStat x
	#define TSE_DECLARE(x) extern CTimerStat x
	#define TSS_DECLARE(x,y) CTimerStatScoped x(y)
	#define TS_BEGIN(x) x.Begin()
	#define TS_END(x) x.End()
	#define TS_RESET(x) x.Reset()
	#define TS_P(x,name) x.Print(name)
	#define TS_PR(x,name) x.Print(name); x.Reset()
	#define TS_EPR(x,name) x.End(); x.Print(name); x.Reset()
#else
	#define TS_DECLARE(x)
	#define TSE_DECLARE(x)
	#define TSS_DECLARE(x,y)
	#define TS_BEGIN(x)
	#define TS_END(x)
	#define TS_RESET(x)
	#define TS_P(x,name)
	#define TS_EPR(x,name)
	#define TS_PR(x,name)
#endif

#endif // FTimerStatH
