#pragma once


#include "Processes.h"
#include "resource.h"
#include "ListAll.h"

ATOM MyRegisterClass(HINSTANCE hInstance);
BOOL InitInstance(HINSTANCE, int);
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
VOID InitializeListViewColumns(HWND);
VOID SetListView (HWND, PProcessList);
VOID ProcessTerminator(HANDLE);
BOOL ProcessCreator(TCHAR*, TCHAR*, TCHAR*);

// Procedimientos de los Dialog Box
INT_PTR CALLBACK AboutDlgProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK CreateDlgProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK EndDlgProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK ModulesDlgProc(HWND, UINT, WPARAM, LPARAM);
VOID ShowModules(HWND);
VOID ShowHeaps(HWND);
VOID ShowThreads(HWND);

// Global Variables:
BOOL bShowWarningDlg = TRUE;
BOOL bShowErrors = FALSE;
HINSTANCE hInst;								// current instance
TCHAR szTitle[MAX_LOADSTRING];					// The title bar text
TCHAR szWindowClass[MAX_LOADSTRING];			// the main window class name
HWND hWnd;										// Handler de la ventana principal

// Averages sizes of letters in default font
static int charXSize = LOWORD (GetDialogBaseUnits());
static int charYSize = HIWORD (GetDialogBaseUnits());

// Listable things
enum ListThing {
	Modules,
	Heaps,
	Threads
};

// referido al proceso del cual se quieren saber modulos, heaps o threads
static DWORD PID; 
// referido a que se quiere saber del proceso
static ListThing ListWhat;

