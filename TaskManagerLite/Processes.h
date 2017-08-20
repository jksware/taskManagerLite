#pragma once

#include "stdafx.h"

typedef struct __tagProcessInfo {
	PROCESSENTRY32 pe32;
	PROCESS_MEMORY_COUNTERS pmc;
} ProcessInfo, *PProcessInfo;

typedef struct __tagProcessList {
	DWORD cProcess;
	PProcessInfo processInfo;
} ProcessList, *PProcessList;

static PProcessList pl;

typedef struct __tagModuleList {
	DWORD cModule;
	MODULEENTRY32 *module;
	BOOL *IsDeviceDriver;
} ModuleList, *PModuleList;

typedef struct __tagHeapList {
	DWORD cHeap;
	HEAPENTRY32 *heap;
} HeapList, *PHeapList;

typedef struct __tagThreadList {
	DWORD cThread;
	THREADENTRY32 *thread;
} ThreadList, *PThreadList;