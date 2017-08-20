#pragma once

#include "stdafx.h"
#include "Processes.h"

using namespace std;

static set<string> sDeviceDrivers;

#define ReserveSize kb(1)

BOOL GetProcessList(HWND, PProcessList);
VOID FreeProcessList(PProcessList);

BOOL ListDeviceDrivers(HWND);

BOOL ListProcessModules(HWND, DWORD, PModuleList);
VOID FreeListProcessModules(PModuleList);

BOOL ListProcessHeaps(HWND, DWORD, PHeapList);
VOID FreeListProcessHeaps(PHeapList);

BOOL ListProcessThreads(HWND, DWORD, PThreadList);
VOID FreeListProcessThreads(PThreadList);

VOID printError(HWND, TCHAR*);