#include "stdafx.h"
#include <malloc.h>
#include <errno.h>


XRCORE_API void vminfo (size_t *_free, size_t *reserved, size_t *committed) {
	MEMORY_BASIC_INFORMATION memory_info;
	memory_info.BaseAddress = 0;
	*_free = *reserved = *committed = 0;
	while (VirtualQuery (memory_info.BaseAddress, &memory_info, sizeof (memory_info))) {
		switch (memory_info.State) {
		case MEM_FREE:
			*_free		+= memory_info.RegionSize;
			break;
		case MEM_RESERVE:
			*reserved	+= memory_info.RegionSize;
			break;
		case MEM_COMMIT:
			*committed += memory_info.RegionSize;
			break;
		}
		memory_info.BaseAddress = (char *) memory_info.BaseAddress + memory_info.RegionSize;
	}
}

XRCORE_API void log_vminfo	()
{
	size_t  w_free, w_reserved, w_committed;
	vminfo	(&w_free, &w_reserved, &w_committed);
	Msg		(
		"* [win32]: free[%d K], reserved[%d K], committed[%d K]",
		w_free/1024,
		w_reserved/1024,
		w_committed/1024
	);
}

int heap_walk (HANDLE heap_handle,struct _heapinfo *_entry)
{
		PROCESS_HEAP_ENTRY Entry;
		DWORD errval;
		int errflag;
		int retval = _HEAPOK;

		Entry.wFlags = 0;
		Entry.iRegionIndex = 0;
		Entry.cbData = 0;
		if ( (Entry.lpData = _entry->_pentry) == NULL ) {
			if ( !HeapWalk( heap_handle, &Entry ) ) {
				if ( GetLastError() == ERROR_CALL_NOT_IMPLEMENTED ) {
					_doserrno = ERROR_CALL_NOT_IMPLEMENTED;
					errno = ENOSYS;
					return _HEAPEND;
				}
				return _HEAPBADBEGIN;
			}
		}
		else {
			if ( _entry->_useflag == _USEDENTRY ) {
				if (!HeapValidate(heap_handle, 0, _entry->_pentry))
					return _HEAPBADNODE;
				Entry.wFlags = PROCESS_HEAP_ENTRY_BUSY;
			}
nextBlock:
			/*
			 * Guard the HeapWalk call in case we were passed a bad pointer
			 * to an allegedly free block.
			 */
			__try {
				errflag = 0;
				if ( !HeapWalk( heap_handle, &Entry ) )
					errflag = 1;
			}
			__except( EXCEPTION_EXECUTE_HANDLER ) {
				errflag = 2;
			}

			/*
			 * Check errflag to see how HeapWalk fared...
			 */
			if ( errflag == 1 ) {
				/*
				 * HeapWalk returned an error.
				 */
				if ( (errval = GetLastError()) == ERROR_NO_MORE_ITEMS ) {
					return _HEAPEND;
				}
				else if ( errval == ERROR_CALL_NOT_IMPLEMENTED ) {
					_doserrno = errval;
					errno = ENOSYS;
					return _HEAPEND;
				}
				return _HEAPBADNODE;
			}
			else if ( errflag == 2 ) {
				/*
				 * Exception occurred during the HeapWalk!
				 */
				return _HEAPBADNODE;
			}
		}

		if ( Entry.wFlags & (PROCESS_HEAP_REGION |
			 PROCESS_HEAP_UNCOMMITTED_RANGE) )
		{
			goto nextBlock;
		}

		_entry->_pentry = (int*)Entry.lpData;
		_entry->_size = Entry.cbData;
		if ( Entry.wFlags & PROCESS_HEAP_ENTRY_BUSY ) {
			_entry->_useflag = _USEDENTRY;
		}
		else {
			_entry->_useflag = _FREEENTRY;
		}

		return( retval );
}

std::string GetLastErrorAsString()
{
	DWORD errorMessageID = ::GetLastError();
	if(errorMessageID == 0)
		return std::string();
	LPSTR messageBuffer = nullptr;
	size_t size = FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
								 nullptr, errorMessageID, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), reinterpret_cast<LPSTR>(&messageBuffer), 0, nullptr);
	std::string message(messageBuffer, size);
	LocalFree(messageBuffer);
	return message;
}

u32	mem_usage_impl	(HANDLE heapHandle, u32* pBlocksUsed, u32* pBlocksFree)
{
	DWORD totalBytes = 0;
	u32	blocksFree	= 0;
	u32	blocksUsed	= 0;
	PROCESS_HEAP_ENTRY heapEntry ={nullptr};
	BOOL locked = HeapLock (heapHandle);
	R_ASSERT3(locked,"HeapLock error!",GetLastErrorAsString().c_str());

#ifdef DEBUG
		if (HeapValidate(heapHandle,0,nullptr)==FALSE)
		{
			FATAL("Invalid heap!");
		}
#endif //DEBUG

	heapEntry.lpData = nullptr;
	while (HeapWalk(heapHandle,&heapEntry)!=FALSE)
	{

		if(heapEntry.wFlags & PROCESS_HEAP_ENTRY_BUSY)
		{
			totalBytes += heapEntry.cbData;
			blocksUsed++;
		}
		else
		{
			blocksFree++;
		}
	}
	HeapUnlock(heapHandle);
	if (GetLastError() != ERROR_NO_MORE_ITEMS ) 
	{
		FATAL2("HeapWalk error!",GetLastErrorAsString().c_str());
	}
		
	if (pBlocksFree)	*pBlocksFree= blocksFree;
	if (pBlocksUsed)	*pBlocksUsed= blocksUsed;
	return totalBytes;
}

u32		xrMemory::mem_usage		(u32* pBlocksUsed, u32* pBlocksFree)
{
	int heapsCount = GetProcessHeaps(0, nullptr);
	if (heapsCount==0)
	{
		FATAL2("Unable get heaps count.",GetLastErrorAsString().c_str());
	}
	HANDLE *heaps = new HANDLE[heapsCount];
	GetProcessHeaps(heapsCount,heaps);
	int allHeapsSize=0;
	for (int i = 0; i < heapsCount; i++)
	{
		u32* bu = nullptr;u32 u=0;bu=&u;
		u32* bf=nullptr;u32 f=0;bf=&f;

		allHeapsSize+=mem_usage_impl(heaps[i],bu,bf);
		if (pBlocksFree)	*pBlocksFree+= *bu;
		if (pBlocksUsed)	*pBlocksUsed+= *bf;
	}
	delete []heaps;
	return allHeapsSize;
}
