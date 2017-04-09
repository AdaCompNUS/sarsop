#include "CPMemUtils.h"

unsigned long getPhysicalMemorySize()
{
#ifdef __APPLE__

	/*
	int mib[2];
	size_t len;
	unsigned long mem_size;
	len = sizeof(mem_size);

	mib[0] = CTL_HW;
	mib[1] = W_PHYSMEM;
	int res = sysctl(mib, 2, &mem_size, &len, NULL, 0);
	if(res == 0)
	{
		return mem_size;
	}
	else
	{
		return 0;
	}
	*/

	return 1024 * 1024 * 1024;
 
#else

#ifdef _MSC_VER
	MEMORYSTATUSEX statex;

	statex.dwLength = sizeof (statex);

	GlobalMemoryStatusEx (&statex);

	return statex.ullTotalPhys;


#else
	//struct sysinfo s_info;
	//sysinfo(&s_info);

	//printf("s_info.totalram %d", s_info.totalram);
	//return ((unsigned long)s_info.totalram) * ((unsigned long)s_info.mem_unit);
	unsigned long int pageSize = sysconf (_SC_PAGESIZE);
	unsigned long int pageNum = sysconf (_SC_PHYS_PAGES);
	
	// HACK: Cygwin running under PAE-enabled windows report page size of 65536
#ifdef __CYGWIN__
	pageSize = 4096;
//	printf("Compiled by Cygwin");
#endif
	//printf("ram size %ld\n", pageSize * pageNum);
	//printf("page size %ld\n", pageSize );
	//printf("page num %ld\n",  pageNum);
	return pageSize * pageNum;

#endif
#endif


}

unsigned long getCurrentProcessMemoryUsage()
{
#ifdef __APPLE__
    // Inspired by:
    // http://miknight.blogspot.com/2005/11/resident-set-size-in-mac-os-x.html
    struct task_basic_info t_info;
    mach_msg_type_number_t t_info_count = TASK_BASIC_INFO_COUNT;
    task_info(current_task(), TASK_BASIC_INFO, (task_info_t)&t_info, &t_info_count);
    size_t size = t_info.resident_size; //: t_info.virtual_size);
    return size;

#else
#ifdef _MSC_VER
	// Open current process
	HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, GetCurrentProcessId());

	if(hProcess)
	{
		PROCESS_MEMORY_COUNTERS ProcessMemoryCounters;

		memset(&ProcessMemoryCounters, 0, sizeof(ProcessMemoryCounters));

		// Set size of structure
		ProcessMemoryCounters.cb = sizeof(ProcessMemoryCounters);

		// Get memory usage
		if(GetProcessMemoryInfo(hProcess,	&ProcessMemoryCounters, sizeof(ProcessMemoryCounters)) == TRUE)
		{
			return ProcessMemoryCounters.WorkingSetSize;
		}
		else
		{
			return 0;
			//::GetLastError()
		}

		// Close process
		CloseHandle(hProcess);
	}
	else
		return 0;
		//std::cout << "Could not open process (Error " << ::GetLastError() << ")" << std::endl;
#else
	struct mallinfo info;

	/* what is the largest ECB heap buffer currently available? */
	info = mallinfo();

	return info.arena;

#endif
#endif



}

unsigned long getPlatformMemoryLimit()
{
#ifdef __APPLE__

#else
#ifdef _MSC_VER


#else

#endif
#endif

	static unsigned long memLimit = -1;
	if(memLimit == -1)
	{
		memLimit = (unsigned long)(getPhysicalMemorySize() * 0.75);
	}
	return memLimit;
}





