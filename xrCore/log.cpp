#include "stdafx.h"
#pragma hdrstop

#include <time.h>
#include <cctype>

#include "resource.h"
#include "log.h"

#include "OPFuncs/ExpandedCmdParams.h"
#include "OPFuncs/utils.h"


extern BOOL					LogExecCB		= TRUE;
static string_path			logFName		= "engine.log";
static BOOL 				no_log			= TRUE;
#ifdef PROFILE_CRITICAL_SECTIONS
	static xrCriticalSection	logCS(MUTEX_PROFILE_ID(log));
#else // PROFILE_CRITICAL_SECTIONS
	static xrCriticalSection	logCS;
#endif // PROFILE_CRITICAL_SECTIONS
xr_vector<shared_str>*		LogFile			= NULL;
static LogCallback			LogCB			= 0;

IWriter *LogWriter;

size_t cached_log = 0;

void FlushLog			(LPCSTR file_name)
{
	if (LogWriter)
		LogWriter->flush();
	cached_log = 0;
}

void FlushLog			()
{
	FlushLog		(logFName);
}

void ClearLogHistory()
{
	if (LogFile)
		LogFile->clear();
}

void AddOne				(const char *split) 
{
	if(!LogFile)		
						return;

	logCS.Enter			();

#ifdef DEBUG
	OutputDebugString	(split);
	OutputDebugString	("\n");
#endif

	//exec CallBack
	if (LogExecCB&&LogCB)LogCB(split);
//	DUMP_PHASE;
	{
		shared_str			temp = shared_str(split);
		LogFile->push_back	(temp);
	}


	//+RvP, alpet
	if (LogWriter)
	{
			switch (*split)
			{
			case 0x21:
			case 0x23:
			case 0x25:
				split ++; // пропустить первый символ, т.к. это вероятно цветовой тег
				break;
			}

			char buf[64]; 
			if (OPFuncs::Dumper->isParamSet(OPFuncs::ExpandedCmdParams::KnownParams::pShowLogTime))
			{
				SYSTEMTIME lt;
				GetLocalTime(&lt);

				sprintf_s(buf, 64, "[%02d.%02d.%02d %02d:%02d:%02d.%03d] ", lt.wDay, lt.wMonth, lt.wYear % 100, lt.wHour, lt.wMinute, lt.wSecond, lt.wMilliseconds);						
			}
			else
			{
				buf[0]=0;
			}
			LogWriter->w_printf("%s%s\r\n", buf, split);
			cached_log += xr_strlen(buf);
			cached_log += xr_strlen(split) + 2;
			if (cached_log >= 16384)
				FlushLog();
		
		//-RvP
	}


	logCS.Leave				();
}

void Log				(const char *s) 
{
	int		i,j;
	char	split[1024];

	for (i=0,j=0; s[i]!=0; i++) {
		if (s[i]=='\n') {
			split[j]=0;	// end of line
			if (split[0]==0) { split[0]=' '; split[1]=0; }
			AddOne(split);
			j=0;
		} else {
			split[j++]=s[i];
		}
	}
	split[j]=0;
	AddOne(split);
}

void __cdecl LogVAList(const char *format, va_list &mark)
{
	string4096	buf; // alpet: размер буфера лучше сделать побольше, чтобы избежать вылетов invalid parameter handler при выводе стеков вызова
	int sz		= _vsnprintf(buf, sizeof(buf)-1, format, mark); buf[sizeof(buf)-1]=0;
	va_end		(mark);
	if (sz)		Log(buf);
}
void __cdecl Msg		( const char *format, ...)
{
	va_list mark;	
	va_start	(mark, format );
	LogVAList   (format, mark);
}

void Log				(const char *msg, const char *dop) {
	char buf[1024];

	if (dop)	sprintf_s(buf,sizeof(buf),"%s %s",msg,dop);
	else		sprintf_s(buf,sizeof(buf),"%s",msg);

	Log		(buf);
}

void Log				(const char *msg, u32 dop) {
	char buf[1024];

	sprintf_s	(buf,sizeof(buf),"%s %d",msg,dop);
	Log			(buf);
}

void Log				(const char *msg, int dop) {
	char buf[1024];

	sprintf_s	(buf, sizeof(buf),"%s %d",msg,dop);
	Log		(buf);
}

void Log				(const char *msg, float dop) {
	char buf[1024];

	sprintf_s	(buf, sizeof(buf),"%s %f",msg,dop);
	Log		(buf);
}

void Log				(const char *msg, const Fvector &dop) {
	char buf[1024];

	sprintf_s	(buf,sizeof(buf),"%s (%f,%f,%f)",msg,dop.x,dop.y,dop.z);
	Log		(buf);
}

void Log				(const char *msg, const Fmatrix &dop)	{
	char	buf	[1024];

	sprintf_s	(buf,sizeof(buf),"%s:\n%f,%f,%f,%f\n%f,%f,%f,%f\n%f,%f,%f,%f\n%f,%f,%f,%f\n",msg,dop.i.x,dop.i.y,dop.i.z,dop._14_
																				,dop.j.x,dop.j.y,dop.j.z,dop._24_
																				,dop.k.x,dop.k.y,dop.k.z,dop._34_
																				,dop.c.x,dop.c.y,dop.c.z,dop._44_);
	Log		(buf);
}

void LogWinErr			(const char *msg, long err_code)	{
	Msg					("%s: %s",msg,Debug.error2string(err_code)	);
}

void SetLogCB			(LogCallback cb)
{
	LogCB				= cb;
}

LPCSTR log_name			()
{
	return				(logFName);
}

void InitLog()
{
	R_ASSERT			(LogFile==NULL);
	LogFile				= xr_new< xr_vector<shared_str> >();
}

void CreateLog			(BOOL nl)
{
	no_log				= nl;
	strconcat			(sizeof(logFName),logFName,Core.ApplicationName,"_",Core.UserName,".log");
	if (FS.path_exist("$logs$"))
		FS.update_path	(logFName,"$logs$",logFName);
	if (!no_log){
		
		HANDLE h=CreateFile(logFName,0,0, nullptr,OPEN_EXISTING,FILE_ATTRIBUTE_READONLY | FILE_FLAG_NO_BUFFERING, nullptr);
		if (h!=INVALID_HANDLE_VALUE )
		{
			FILETIME creationTime,lastAccessTime,lastWriteTime;
			int result=GetFileTime( h, &creationTime, &lastAccessTime, &lastWriteTime );
			R_ASSERT3(result,"Can't obtain information about file:",logFName);
			CloseHandle(h);

			SYSTEMTIME systemTime;
			FILETIME localFiletime;
			result=FileTimeToLocalFileTime(&lastWriteTime, &localFiletime);
			R_ASSERT2(result,"Can't convert FILETIME to LOCALFILETIME.");
			result = FileTimeToSystemTime( &localFiletime, &systemTime );
			R_ASSERT2(result,"Can't convert LOCALTIME to SYSTEMTIME.");
			
			string1024 strTime;
			sprintf_s(strTime,"%04d_%02d_%02d-%02d_%02d_%02d",systemTime.wYear,systemTime.wMonth,systemTime.wDay,systemTime.wHour,systemTime.wMinute,systemTime.wSecond);
			string1024 newLogFileName;
			sprintf_s(newLogFileName,"%s_%s_%s.log",OPFuncs::tolower(Core.ApplicationName).c_str(),OPFuncs::tolower(Core.UserName).c_str(),strTime);
			//std::string strTime=shared_str().sprintf("%04d_%02d_%02d-%02d_%02d_%02d",systemTime.wYear,systemTime.wMonth,systemTime.wDay,systemTime.wHour,systemTime.wMinute,systemTime.wSecond).c_str();
			//std::string	newLogFileName=shared_str().sprintf("%s_%s_%s.log",OPFuncs::tolower(Core.ApplicationName).c_str(),OPFuncs::tolower(Core.UserName).c_str(),strTime).c_str();
			string_path fullNewLogFileName;
			FS.update_path	(fullNewLogFileName,"$logs$",newLogFileName);
			MoveFile(logFName,fullNewLogFileName);
		}
		
		LogWriter = FS.w_open	(logFName);
		if (LogWriter==NULL){
			MessageBox	(NULL,"Can't create log file.","Error",MB_ICONERROR);
			abort();
		}
		
		char buf[64];
		if (OPFuncs::Dumper->isParamSet(OPFuncs::ExpandedCmdParams::KnownParams::pShowLogTime))
		{
			time_t t = time(NULL);
			tm* ti = localtime(&t);
			strftime(buf, 64, "[%x %X]\t", ti);
		}
		else	
		{
			buf[0]=0;
		}
		for (u32 it=0; it<LogFile->size(); it++)	{
			LPCSTR		s	= *((*LogFile)[it]);
			LogWriter->w_printf("%s%s\n", buf, s?s:"");
		}
		LogWriter->flush();
	}
	LogFile->reserve		(128);
}

void CloseLog(void)
{
	if(LogWriter){
		FS.w_close(LogWriter);
	}

	FlushLog		();
	LogFile->clear	();
	xr_delete		(LogFile);
}
