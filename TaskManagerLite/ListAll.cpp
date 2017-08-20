#include "stdafx.h"
#include "ListAll.h"

BOOL GetProcessList(HWND hWnd, PProcessList pPL)
{
	HANDLE hProcessSnap;
	HANDLE hProcess;
	DWORD dwPriorityClass;

	pPL->cProcess = 0;
	pPL->processInfo = new ProcessInfo[ReserveSize];
	ZeroMemory(pPL->processInfo, ReserveSize * sizeof(ProcessInfo));

	hProcessSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	if(hProcessSnap == INVALID_HANDLE_VALUE)
	{
		printError(hWnd, _T("CreateToolhelp32Snapshot (of processes)"));
		return FALSE;
	}

	pPL->processInfo[0].pe32.dwSize = sizeof(PROCESSENTRY32);

	// Retrieve information about the first process,
	// and exit if unsuccessful
	if (!Process32First(hProcessSnap, &pPL->processInfo[0].pe32))
	{
		printError(hWnd, _T("Process32First"));
		CloseHandle(hProcessSnap);
		return FALSE;
	}

	int i = 0;
	do
	{
		pPL->processInfo[i].pmc.cb = sizeof(PROCESS_MEMORY_COUNTERS);
		BOOL bProcessMemoryInfoOk = FALSE;

		// Retrieve the priority class.
		dwPriorityClass = 0;
		hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE,
			pPL->processInfo[i].pe32.th32ProcessID);
		if (hProcess)
		{
			GetProcessMemoryInfo(hProcess, &pPL->processInfo[i].pmc, 
				sizeof(PROCESS_MEMORY_COUNTERS));

			CloseHandle(hProcess);
		}

		pPL->processInfo[++i].pe32.dwSize = sizeof(PROCESSENTRY32);
	} while (Process32Next(hProcessSnap, &pPL->processInfo[i].pe32));

	pPL->cProcess = i;

	CloseHandle(hProcessSnap);
	return TRUE;
}

VOID FreeProcessList(PProcessList pPL)
{
	delete pPL->processInfo;
	delete pPL;
}

BOOL ListDeviceDrivers(HWND hWnd)
{
	LPVOID drivers[ReserveSize];
	DWORD dwNeeded;
	BOOL bEnumDriverOk = EnumDeviceDrivers(drivers, sizeof(drivers), 
		&dwNeeded) && dwNeeded < sizeof(drivers);
	int	cDriver = dwNeeded / sizeof(drivers[0]);

	if (!bEnumDriverOk)
	{
		printError(hWnd, _T("EnumDeviceDrivers"));
		return FALSE;
	}

	TCHAR szDriver[MAX_PATH];

	for (int j = 0; j < cDriver; j++)
		if (GetDeviceDriverBaseName(drivers[j], szDriver, sizeof(szDriver)))
			sDeviceDrivers.insert(szDriver);
		else
			printError(hWnd, _T("GetDeviceDriverBaseName"));

	return TRUE;
}

BOOL ListProcessModules(HWND hWnd, DWORD dwPID, PModuleList pML)
{
	pML->cModule = 0;
	pML->module = new MODULEENTRY32[ReserveSize];
	pML->IsDeviceDriver = new BOOL[ReserveSize];
	ZeroMemory(pML->module, ReserveSize * sizeof(MODULEENTRY32));
	ZeroMemory(pML->IsDeviceDriver, ReserveSize * sizeof(BOOL));

	HANDLE hModuleSnap = INVALID_HANDLE_VALUE;

	hModuleSnap = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE | 
		TH32CS_SNAPMODULE32, dwPID);
	if(hModuleSnap == INVALID_HANDLE_VALUE)
	{
		printError(hWnd, _T("CreateToolhelp32Snapshot (of modules)"));
		return FALSE;
	}

	pML->module[0].dwSize = sizeof(MODULEENTRY32);

	if (!Module32First(hModuleSnap, &pML->module[0]))
	{
		printError(hWnd, _T("Module32First"));
		CloseHandle(hModuleSnap);
		return FALSE;
	}
	
	int i = 0;

	do
	{
		pML->IsDeviceDriver[i] = (sDeviceDrivers.find(pML->module[i].szModule) != sDeviceDrivers.end());
		pML->module[++i].dwSize = sizeof(MODULEENTRY32);
	} while (Module32Next(hModuleSnap, &pML->module[i]));

	pML->cModule = i;

	CloseHandle(hModuleSnap);
	return TRUE;
}

VOID FreeListProcessModules(PModuleList pML)
{
	delete pML->module;
	delete pML->IsDeviceDriver;
	delete pML;
}

BOOL ListProcessHeaps(HWND hWnd, DWORD dwPID, PHeapList pHL)
{
	pHL->cHeap = 0;
	pHL->heap = new HEAPENTRY32[ReserveSize];
	ZeroMemory(pHL->heap, ReserveSize * sizeof(HEAPENTRY32));

	HANDLE hHeapList = INVALID_HANDLE_VALUE;
	HEAPLIST32 hl32;

	hHeapList = CreateToolhelp32Snapshot(TH32CS_SNAPHEAPLIST, dwPID);
	if(hHeapList == INVALID_HANDLE_VALUE)
	{
		printError(hWnd, _T("CreateToolhelp32Snapshot (of heaps)"));
		return FALSE;
	}

	hl32.dwSize = sizeof(HEAPLIST32);

	if (!Heap32ListFirst(hHeapList, &hl32))
	{
		printError(hWnd, _T("Heap32ListFirst"));
		CloseHandle(hHeapList);
		return FALSE;
	}

	int i = 0;
	pHL->heap[0].dwSize = sizeof(HEAPENTRY32);

	do
	{
		if (Heap32First(&pHL->heap[i], hl32.th32ProcessID, hl32.th32HeapID))
		{
			do
			{
				pHL->heap[++i].dwSize = sizeof(HEAPENTRY32);
			} while (Heap32Next(&pHL->heap[i]));
		}
		hl32.dwSize = sizeof(HEAPLIST32);

	} while (Heap32ListNext(hHeapList, &hl32));

	pHL->cHeap = i;

	CloseHandle(hHeapList);
	return TRUE;
}

VOID FreeListProcessHeaps(PHeapList pHL)
{
	delete pHL->heap;
	delete pHL;
}

BOOL ListProcessThreads(HWND hWnd, DWORD dwPID, PThreadList pTL) 
{ 
	pTL->cThread = 0;
	pTL->thread = new THREADENTRY32[ReserveSize];
	ZeroMemory(pTL->thread, ReserveSize * sizeof(THREADENTRY32));

	HANDLE hThreadSnap = INVALID_HANDLE_VALUE; 
	THREADENTRY32 te32; 

	hThreadSnap = CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, 0); 
	if( hThreadSnap == INVALID_HANDLE_VALUE ) 
		return( FALSE ); 

	te32.dwSize = sizeof(THREADENTRY32); 

	if (!Thread32First(hThreadSnap, &te32)) 
	{
		printError(hWnd, _T("Thread32First"));
		CloseHandle(hThreadSnap);
		return FALSE;
	}

	int i = 0;

	do 
	{ 
		if (te32.th32OwnerProcessID == dwPID)
		{
			memcpy(&pTL->thread[i++], &te32, sizeof(THREADENTRY32));
		}
	} while (Thread32Next(hThreadSnap, &te32));

	pTL->cThread = i;

	CloseHandle(hThreadSnap);
	return TRUE;
}

VOID FreeListProcessThreads(PThreadList pTL)
{
	delete pTL->thread;
	delete pTL;
}

VOID printError(HWND hWnd, TCHAR* msg)
{
	DWORD eNum;
	TCHAR sysMsg[256];
	TCHAR* p;

	eNum = GetLastError( );
	FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
		NULL, eNum,
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		sysMsg, 256, NULL);

	p = sysMsg;
	while ((*p > 31) || (*p == 9))
		++p;
	do { *p-- = 0; }
	while ((p >= sysMsg) && ((*p == '.') || (*p < 33)));

	TCHAR szBuffer[kb(1)];
	_stprintf_s(szBuffer, _T("%s falló con error %d (%s)\n"), msg, eNum, sysMsg);
	
	MessageBox(hWnd, szBuffer, _T("TaskManagerLite - Error"), MB_OK | MB_ICONERROR);
}
