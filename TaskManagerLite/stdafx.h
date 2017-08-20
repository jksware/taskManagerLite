// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

#include "targetver.h"

#define _WIN32_IE 0x0600
#define WINVER 0x501

// para los temas
#if defined _M_IX86
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='x86' publicKeyToken='6595b64144ccf1df' language='*'\"")
#elif defined _M_IA64
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='ia64' publicKeyToken='6595b64144ccf1df' language='*'\"")
#elif defined _M_X64
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='amd64' publicKeyToken='6595b64144ccf1df' language='*'\"")
#else
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")
#endif

//#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers
// Windows Header Files:
#include <Windows.h>
#include <commctrl.h>
#include <Uxtheme.h>
#include <dwmapi.h>
#include <TlHelp32.h>
#include <Psapi.h>
#include <wincred.h>
#include <UserEnv.h>
#include <WinUser.h>
#include <WindowsX.h>

// CRT Header Files
#include <stdlib.h>
#include <malloc.h>
#include <memory.h>
#include <tchar.h>

#include <set>
#include <string>

#define kb(x) ((x)*1024)
#define sizeOfPA (kb(1) * sizeof(DWORD))

// create button
#define IDB_CREATE 0XFFFB
// end button
#define IDB_END 0xFFFA

#define MAX_LOADSTRING 100

// Default window margin
#define CBTN_MARGIN 10
// Default button width
#define CBTN_X 85
// Default button height
#define CBTN_Y 25
// Timer interval message
#define TIMER_INTERVAL 1000