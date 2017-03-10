#ifndef FTimerStatH
#define FTimerStatH
#pragma once

#include "FTimer.h"
#include <vector>

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

class XRCORE_API CTimerStatScopedNamed
{
public:
	typedef void (*fn)(const char* name, const char* enabler);
public:
	CTimerStatScopedNamed(const char* name, const char* enabler, fn begin, fn end) : m_name(name), m_enabler(enabler), m_end(end) { begin(name, enabler); }
	~CTimerStatScopedNamed() { m_end(m_name, m_enabler); }
private:
	CTimerStatScopedNamed(const CTimerStatScopedNamed& rTimer);
	CTimerStatScopedNamed& operator=(const CTimerStatScopedNamed& rTimer);
	const char* m_name;
	const char* m_enabler;
	fn m_end;
};

#ifdef TS_ENABLE
	XRCORE_API void TSM_Init();
	XRCORE_API void TSM_DeInit();
	XRCORE_API void TSM_Enable(const char* enabler);
	XRCORE_API void TSM_Disable(const char* enabler);
	XRCORE_API void TSM_BeginProfile(const char* name, const char* enabler);
	XRCORE_API void TSM_EndProfile(const char* name, const char* enabler);
	XRCORE_API void TSM_Begin(const char* name, const char* enabler);
	XRCORE_API void TSM_End(const char* name, const char* enabler);
	XRCORE_API void TSM_Print();
	XRCORE_API void TSM_Print(const char* name);

	#define TSE_INIT() TSM_Init()
	#define TSE_DEINIT() TSM_DeInit()
	#define TSE_ENABLE(e) TSM_Enable(e)
	#define TSE_DISABLE(e) TSM_Disable(e)
	#define TSP_BEGIN(n, e) TSM_BeginProfile(n, e)
	#define TSP_END(n, e) TSM_EndProfile(n, e)
	#define TSP_SCOPED(s,n,e) CTimerStatScopedNamed s(n, e, TSM_BeginProfile, TSM_EndProfile);
	#define TSS_BEGIN(n, e) TSM_Begin(n, e)
	#define TSS_END(n, e) TSM_End(n, e)
	#define TSS_SCOPED(s,n,e) CTimerStatScopedNamed s(n, e, TSM_Begin, TSM_End);
	#define TSP_PRINT() TSM_Print()
	#define TSS_PRINT(n) TSM_Print(n)
#else
	#define TSE_INIT()
	#define TSE_DEINIT()
	#define TSE_DISABLE(e)
	#define TS_ENABLE(e)
	#define TS_DISABLE(e)
	#define TSP_BEGIN(n, e)
	#define TSP_END(n, e)
	#define TSP_SCOPED(s,n,e)
	#define TSS_BEGIN(n, e)
	#define TSS_END(n, e)
	#define TSS_SCOPED(s,n,e)
	#define TSP_PRINT()
	#define TSS_PRINT(n)
#endif

#endif // FTimerStatH
