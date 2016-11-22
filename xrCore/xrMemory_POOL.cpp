#include "stdafx.h"
#pragma hdrstop

#include "xrMemory_POOL.h"
#include "xrMemory_align.h"

extern LPCSTR BuildStackTrace();
extern char g_stackTrace[100][4096];
extern int	g_stackTraceCount;
size_t g_stackTraceLengths[100];

std::hash_map<size_t, char*> g_stackTraces;
std::vector<size_t> g_blocks;

void	MEMPOOL::block_create	()
{
	// Allocate
	R_ASSERT				(0==list);
	list					= (u8*)		xr_aligned_offset_malloc	(s_sector,16,s_offset);

	// Partition
	for (u32 it=0; it<(s_count-1); it++)
	{
		u8*	E				= list + it*s_element;
		*access(E)			= E+s_element;
	}
	*access	(list+(s_count-1)*s_element)	= NULL;
	block_count				++;

	if (s_element == 480)
	{
		g_blocks.push_back((size_t)list);
	}
}

void	MEMPOOL::_initialize	(u32 _element, u32 _sector, u32 _header)
{
	R_ASSERT		(_element < _sector/2);
	s_sector		= _sector;
	s_element		= _element;
	s_count			= s_sector/s_element;
	s_offset		= _header;
	list			= NULL;
	block_count		= 0;
}

void MEMPOOL::store_stat(void* e)
{
	if (s_element != 480) return;

	BuildStackTrace();
	size_t stackTraceLen = 0;

	for (int i = 0; i < g_stackTraceCount; i++)
	{
		g_stackTraceLengths[i] = strlen(g_stackTrace[i]);
		stackTraceLen += g_stackTraceLengths[i] + 1;
	}

	char* stackTrace  = new char[stackTraceLen+1];
	size_t offset = 0;
	for (int i = 0; i < g_stackTraceCount; i++)
	{
		memcpy(stackTrace + offset, g_stackTrace[i], g_stackTraceLengths[i]);
		offset += g_stackTraceLengths[i];
		stackTrace[offset] = '\n';
		offset += 1;
	}
	stackTrace[offset] = 0;
	g_stackTraces[(size_t)e] = stackTrace;
}

void MEMPOOL::remove_stat(void* e)
{
	if (s_element != 480) return;

	auto i = g_stackTraces.find((size_t)e);
	delete [] i->second;
	g_stackTraces.erase(i);
}

void MEMPOOL::_dump(const void* corrupted_memory_item_ptr)
{
	string_path dump_name;
	if (FS.path_exist("$logs$"))
		FS.update_path	(dump_name,"$logs$","custom_data_corrupt_info.txt");
	else
		strcpy(dump_name, "C:\\custom_data_corrupt_info.txt");

	FILE* f		= fopen(dump_name,"w");
	if (!f) return;

	fprintf(f, "Corrupted ptr: %p\n", corrupted_memory_item_ptr);

	std::vector<std::vector<char*>> table;
	table.resize(g_blocks.size());
	for (size_t i = 0; i < table.size(); ++i)
	{
		table[i].resize(s_count, NULL);
	}

	for (auto i = g_stackTraces.begin(); i != g_stackTraces.end(); ++i)
	{
		size_t element = i->first;
		char* stack = i->second;
		for (size_t b = 0; b < g_blocks.size(); ++b)
		{
			if (g_blocks[b] <= element  && element < g_blocks[b] + s_sector)
			{
				table[b][(element - g_blocks[b])/s_element] = stack;
				break;
			}
		}
	}

	fprintf(f, "Block table\n");
	size_t cb = 0, cbi = 0;
	for (size_t b = 0; b < table.size(); ++b)
	{
		fprintf(f, "Block %2u [%p-%p]: [", b, g_blocks[b], g_blocks[b] + s_sector);
		std::vector<char*>& rstacks = table[b];
		for (size_t bi = 0; bi < rstacks.size(); bi++)
		{
			char c;
			if ((g_blocks[b] + bi*s_element) <= (size_t)corrupted_memory_item_ptr 
				&&
				(size_t)corrupted_memory_item_ptr <= (g_blocks[b] + (bi+1)*s_element))
			{
				c = '*';
				cb = b;
				cbi = bi;
			}
			else
			{
				c = rstacks[bi] == NULL ? 'F' : 'U';
			}
			fprintf(f, "%c",  c);
		}
		fprintf(f, "]\n");
	}

	fprintf(f, "Allocated items: %u\n", g_stackTraces.size());
	for (size_t b = 0; b < table.size(); ++b)
	{
		std::vector<char*>& rstacks = table[b];
		for (size_t bi = 0; bi < rstacks.size(); bi++)
		{
			fprintf(f, "Item%s[%3u:%2u]\n%s\n", ((cb == b && cbi == bi) ? "*" : "" ), b, bi, rstacks[bi]);
		}
	}

	fclose(f);
}
