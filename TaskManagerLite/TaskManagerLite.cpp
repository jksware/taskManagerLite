#include "stdafx.h"
#include "TaskManagerLite.h"
#include "Utils.h"

#pragma warning (disable: 4996)

int APIENTRY _tWinMain(_In_ HINSTANCE hInstance,
					   _In_opt_ HINSTANCE hPrevInstance,
					   _In_ LPTSTR    lpCmdLine,
					   _In_ int       nCmdShow)
{	
	INITCOMMONCONTROLSEX icex;
	icex.dwICC = ICC_BAR_CLASSES | ICC_LISTVIEW_CLASSES | ICC_NATIVEFNTCTL_CLASS | ICC_STANDARD_CLASSES;
	icex.dwSize = sizeof(icex);
	InitCommonControlsEx(&icex);

	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);

	MSG msg;
	HACCEL hAccelTable;

	LoadString(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
	LoadString(hInstance, IDC_TASKMANAGERLITE, szWindowClass, MAX_LOADSTRING);
	MyRegisterClass(hInstance);

	if (!InitInstance (hInstance, nCmdShow))
	{
		return FALSE;
	}

	hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_TASKMANAGERLITE));

	while (GetMessage(&msg, NULL, 0, 0))
	{
		if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}

	return (int) msg.wParam;
}

ATOM MyRegisterClass(HINSTANCE hInstance)
{
	WNDCLASSEX wcex;

	wcex.cbSize = sizeof(WNDCLASSEX);

	wcex.style			= CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc	= WndProc;
	wcex.cbClsExtra		= 0;
	wcex.cbWndExtra		= 0;
	wcex.hInstance		= hInstance;
	wcex.hIcon			= LoadIcon(hInstance, MAKEINTRESOURCE(IDI_TASKMANAGERLITE));
	wcex.hCursor		= LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground	= (HBRUSH)(COLOR_BTNFACE+1);
	wcex.lpszMenuName	= MAKEINTRESOURCE(IDC_TASKMANAGERLITE);
	wcex.lpszClassName	= szWindowClass;
	wcex.hIconSm		= LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

	return RegisterClassEx(&wcex);
}

BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
	hInst = hInstance;

	hWnd = CreateWindowEx(
#ifdef _DEBUG
		NULL,
#else
		WS_EX_TOPMOST,
#endif
		szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, 0, 490, 600, NULL, NULL, hInstance, NULL);

	if (!hWnd)
	{
		return FALSE;
	}

	ShowWindow(hWnd, nCmdShow);
	UpdateWindow(hWnd);

	return TRUE;
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	int wmId, wmEvent;

	static HWND hwndBtnEnd;
	static HWND hwndBtnCreate;
	static HWND hwndListView;
	static HWND hwndListViewM ;
	static HWND hwndLabelProcess;
	static HWND hwndStatusBar;

	PAINTSTRUCT ps;
	HDC hdc;
	RECT rect = {0, 0, 0, 0};

	switch (message)
	{
#pragma region Create
	case WM_CREATE:
		{
			GetClientRect(hWnd, &rect);

			hwndBtnCreate = CreateWindow(
				WC_BUTTON,
				_T("Crear"),
				WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON | BS_TEXT | WS_TABSTOP,
				CBTN_MARGIN,											// x coord
				rect.bottom - 3 * CBTN_MARGIN - CBTN_Y,					// y coord
				CBTN_X, 												// cx
				CBTN_Y,													// cy
				hWnd, (HMENU) IDB_CREATE,
				hInst, NULL);	

			hwndBtnEnd = CreateWindow(
				WC_BUTTON,
				_T("Terminar"),
				WS_CHILD | WS_VISIBLE | BS_DEFPUSHBUTTON | BS_TEXT,
				rect.right - CBTN_MARGIN - CBTN_X,						// x coord
				rect.bottom - 3 * CBTN_MARGIN - CBTN_Y,					// y coord
				CBTN_X,													// cx
				CBTN_Y,													// cy
				hWnd,
				(HMENU) IDB_END,
				hInst, NULL);

			hwndListView = CreateWindow(
				WC_LISTVIEW,
				_T(""),
				WS_CHILD | WS_VISIBLE | LVS_REPORT | WS_BORDER | WS_TABSTOP |
				LVS_SINGLESEL | LVS_SHOWSELALWAYS | WS_VSCROLL | WS_HSCROLL,
				CBTN_MARGIN,											// x coord
				CBTN_MARGIN,											// y coord
				rect.right - 2 * CBTN_MARGIN,							// cx
				rect.bottom - 7 * CBTN_MARGIN,							// cy
				hWnd,
				(HMENU) 4,
				hInst, NULL);

			ListView_SetExtendedListViewStyle(hwndListView,
				LVS_EX_DOUBLEBUFFER | LVS_EX_UNDERLINEHOT |
				LVS_EX_FULLROWSELECT | LVS_EX_ONECLICKACTIVATE);

			hwndStatusBar = CreateWindow(
				STATUSCLASSNAME,
				_T("Listo"),
				SBARS_SIZEGRIP | WS_CHILD | WS_VISIBLE,
				0, 0, 0, 0,
				hWnd,
				(HMENU) NULL,
				hInst, NULL);

			InitializeListViewColumns(hwndListView);

			pl = new ProcessList();
			GetProcessList(hWnd, pl);
			SetListView(hwndListView, pl);
			FreeProcessList(pl);

			EnumChildWindows(hWnd, SetChildFont, (LPARAM)GetMessageFont());

			ListDeviceDrivers(hWnd);

			SetTimer(hWnd, 1, TIMER_INTERVAL, NULL);
			break;
		}
#pragma endregion

	case WM_TIMER:
		SendMessage(hwndListView, WM_SETREDRAW, FALSE, NULL);

		pl = new ProcessList();
		GetProcessList(hWnd, pl);
		SetListView(hwndListView, pl);

		TCHAR szBuffer[MAX_PATH];
		wsprintf(szBuffer, _T("%d procesos."), pl->cProcess);
		SendMessage(
			hwndStatusBar,
			SB_SETTEXT,
			MAKEWORD(0, 0) & 0xFFFF,
			(LPARAM)szBuffer);

		FreeProcessList(pl);

		SendMessage(hwndListView, WM_SETREDRAW, TRUE, NULL);

		break;

	case WM_SIZE:
		{
			DWORD dwNewWidth = LOWORD(lParam);
			DWORD dwNewHeight = HIWORD(lParam);

			MoveWindow(
				hwndBtnCreate, 
				CBTN_MARGIN,											// x coord
				dwNewHeight - 3 * CBTN_MARGIN - CBTN_Y,					// y coord
				CBTN_X, 												// cx
				CBTN_Y,													// cy
				TRUE);

			MoveWindow(
				hwndBtnEnd,
				dwNewWidth - CBTN_MARGIN - CBTN_X,						// x coord
				dwNewHeight - 3 * CBTN_MARGIN - CBTN_Y,					// y coord
				CBTN_X,													// cx
				CBTN_Y,													// cy
				TRUE);

			MoveWindow(
				hwndListView,
				CBTN_MARGIN,											// x coord
				CBTN_MARGIN,											// y coord
				dwNewWidth - 2 * CBTN_MARGIN,							// cx
				dwNewHeight - 7 * CBTN_MARGIN,							// cy
				TRUE);

			SendMessage(hwndStatusBar, WM_SIZE, lParam, wParam);
		}
		break;

	case WM_WINDOWPOSCHANGING:
		{
			PWINDOWPOS pWindowPos = (PWINDOWPOS)lParam;

			/*if (pWindowPos->cx < 200 || pWindowPos->cy < 150)
			pWindowPos->flags |= SWP_NOSIZE;*/

			if (pWindowPos->cx < 250)
				pWindowPos->cx = 250;

			if (pWindowPos->cy < 400)
				pWindowPos->cy = 400;
		}
		break;

#pragma region Command
	case WM_COMMAND:
		wmId    = LOWORD(wParam);
		wmEvent = HIWORD(wParam);
		switch (wmId)
		{
		case IDM_ABOUT:
			DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, AboutDlgProc);
			break;

		case IDM_EXIT:
			DestroyWindow(hWnd);
			break;

		case IDM_MOSTRARADVERTENCIA:
			if (!bShowWarningDlg)
				bShowWarningDlg = true;
			break;

		case IDM_CREARPROCESO:
		case IDB_CREATE:
			{
				int answer = DialogBox(hInst, MAKEINTRESOURCE(IDD_CREATE), hWnd, CreateDlgProc);
				if (!answer)
					MessageBox(hWnd, _T("No se pudo crear el proceso."), szTitle, MB_OK | MB_ICONERROR);
			}
			break;

		case IDM_ELIMINARPROCESO:
		case IDVK_DEL:
		case IDB_END:
			{						
				int iPID = GetSelectionPID(hwndListView);

				if (iPID == -1)
				{
					MessageBox(hWnd, _T("Debe seleccionar un proceso para matar."), szTitle, MB_OK | MB_ICONEXCLAMATION);
					break;
				}
				
				if (bShowWarningDlg)
				{
					int answer = DialogBox(hInst, MAKEINTRESOURCE(IDD_ENDPROC), hWnd, EndDlgProc);
					if (answer == IDYES)
						ProcessTerminator(OpenProcess(PROCESS_TERMINATE, FALSE, iPID));
				}
				else
				{
					ProcessTerminator(OpenProcess(PROCESS_TERMINATE, FALSE, iPID));	
				}

				break;
			}

		case IDM_VERMODULOS:
			ShowModules(hwndListView);
			break;

		case IDM_VERHEAPS:
			ShowHeaps(hwndListView);
			break;

		case IDM_VERHILOS:
			ShowThreads(hwndListView);
			break;
			
		default:
			return DefWindowProc(hWnd, message, wParam, lParam);
		}
		break;
#pragma endregion

	case WM_NOTIFY:
		switch (((LPNMHDR)lParam)->code)
		{
		case NM_DBLCLK:
			if (((LPNMHDR)lParam)->hwndFrom == hwndListView)
			{
				ShowModules(hwndListView);
			}
			break; 
		}
		break;

	case WM_PAINT:
		{
			hdc = BeginPaint(hWnd, &ps);
			GetClientRect(hWnd, &rect);
			EndPaint(hWnd, &ps);

			break;
		}

	case WM_DESTROY:
		PostQuitMessage(0);
		break;

	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}

VOID ShowModules(HWND hwnd)
{
	PID = GetSelectionPID(hwnd);
	ListWhat = Modules;

	DialogBox(hInst, MAKEINTRESOURCE(IDD_MODULESBOX), hWnd, ModulesDlgProc);
}

VOID ShowHeaps(HWND hwnd)
{
	PID = GetSelectionPID(hwnd);
	ListWhat = Heaps;

	DialogBox(hInst, MAKEINTRESOURCE(IDD_MODULESBOX), hWnd, ModulesDlgProc);
}

VOID ShowThreads(HWND hwnd)
{
	PID = GetSelectionPID(hwnd);
	ListWhat = Threads;

	DialogBox(hInst, MAKEINTRESOURCE(IDD_MODULESBOX), hWnd, ModulesDlgProc);
}

VOID InitializeListViewColumns(HWND hListView)
{
	int nColumn = 0;

	LVCOLUMN column;
	column.mask = LVCF_WIDTH | LVCF_TEXT;
	column.pszText = _T("Name");
	column.cx = charXSize * 21;
	ListView_InsertColumn(hListView, nColumn++, &column);

	column.mask = LVCF_WIDTH | LVCF_TEXT | LVCF_FMT;
	column.pszText = _T("PID");
	column.cx = charXSize * 10;
	ListView_InsertColumn(hListView, nColumn++, &column);

	column.pszText = _T("Parent PID");
	column.cx = charXSize * 10;
	ListView_InsertColumn(hListView, nColumn++, &column);

	column.pszText = _T("Thread count");
	column.cx = charXSize * 10;
	column.fmt = LVCFMT_RIGHT;
	ListView_InsertColumn(hListView, nColumn++, &column);

	column.pszText = _T("Priority base");
	column.cx = charXSize * 10;
	ListView_InsertColumn(hListView, nColumn++, &column);

	column.pszText = _T("Page Fault Count");
	column.cx = charXSize * 10;
	ListView_InsertColumn(hListView, nColumn++, &column);

	column.pszText = _T("Peak Working Set Size");
	column.cx = charXSize * 10;
	ListView_InsertColumn(hListView, nColumn++, &column);

	column.pszText = _T("Working Set Size");
	column.cx = charXSize * 10;
	ListView_InsertColumn(hListView, nColumn++, &column);

	column.pszText = _T("Quota Peak Paged Pool Usage");
	column.cx = charXSize * 10;
	ListView_InsertColumn(hListView, nColumn++, &column);

	column.pszText = _T("Quota Paged Pool Usage");
	column.cx = charXSize * 10;
	ListView_InsertColumn(hListView, nColumn++, &column);

	column.pszText = _T("Quota Peak Non Paged Pool Usage");
	column.cx = charXSize * 10;
	ListView_InsertColumn(hListView, nColumn++, &column);

	column.pszText = _T("Quota Non Paged Pool Usage");
	column.cx = charXSize * 10;
	ListView_InsertColumn(hListView, nColumn++, &column);

	column.pszText = _T("Pagefile Usage");
	column.cx = charXSize * 10;
	ListView_InsertColumn(hListView, nColumn++, &column);

	column.pszText = _T("Peak Pagefile Usage");
	column.cx = charXSize * 10;
	ListView_InsertColumn(hListView, nColumn++, &column);
}

VOID SetListView(HWND hListView, PProcessList plView)
{
	int cItems = ListView_GetItemCount(hListView);

	TCHAR szBuffer[MAX_PATH];

	LVITEM item;
	ZeroMemory(&item, sizeof(item));
	item.mask = LVIF_TEXT;
	item.pszText = szBuffer;

	for (int i = 0; i <= plView->cProcess; i++)
	{
		item.iItem = i;
		int cSubItem = 0;
		item.iSubItem = cSubItem++;
		_tcscpy(item.pszText, plView->processInfo[i].pe32.szExeFile);
		if (i > cItems)
			ListView_InsertItem(hListView, &item);
		else
			ListView_SetItem(hListView, &item);

		item.iSubItem = cSubItem++;
		_itot(plView->processInfo[i].pe32.th32ProcessID, item.pszText, 10);
		ListView_SetItem(hListView, &item);

		item.iSubItem = cSubItem++;
		_itot(plView->processInfo[i].pe32.th32ParentProcessID, item.pszText, 10);
		ListView_SetItem(hListView, &item);

		item.iSubItem = cSubItem++;
		_itot(plView->processInfo[i].pe32.cntThreads, item.pszText, 10);
		ListView_SetItem(hListView, &item);

		item.iSubItem = cSubItem++;
		_itot(plView->processInfo[i].pe32.pcPriClassBase, item.pszText, 10);
		ListView_SetItem(hListView, &item);

		item.iSubItem = cSubItem++;
		_itot(plView->processInfo[i].pmc.PageFaultCount, item.pszText, 10);
		ListView_SetItem(hListView, &item);

		item.iSubItem = cSubItem++;
		wsprintf(item.pszText, _T("%d K"), plView->processInfo[i].pmc.PeakWorkingSetSize / kb(1));
		ListView_SetItem(hListView, &item);

		item.iSubItem = cSubItem++;
		wsprintf(item.pszText, _T("%d K"), plView->processInfo[i].pmc.WorkingSetSize / kb(1));
		ListView_SetItem(hListView, &item);

		item.iSubItem = cSubItem++;
		wsprintf(item.pszText, _T("%d K"), plView->processInfo[i].pmc.QuotaPeakPagedPoolUsage / kb(1));
		ListView_SetItem(hListView, &item);

		item.iSubItem = cSubItem++;
		wsprintf(item.pszText, _T("%d K"), plView->processInfo[i].pmc.QuotaPagedPoolUsage / kb(1));
		ListView_SetItem(hListView, &item);

		item.iSubItem = cSubItem++;
		wsprintf(item.pszText, _T("%d K"), plView->processInfo[i].pmc.QuotaPeakNonPagedPoolUsage / kb(1));
		ListView_SetItem(hListView, &item);

		item.iSubItem = cSubItem++;
		wsprintf(item.pszText, _T("%d K"), plView->processInfo[i].pmc.QuotaNonPagedPoolUsage / kb(1));
		ListView_SetItem(hListView, &item);

		item.iSubItem = cSubItem++;
		wsprintf(item.pszText, _T("%d K"), plView->processInfo[i].pmc.PagefileUsage / kb(1));
		ListView_SetItem(hListView, &item);

		item.iSubItem = cSubItem++;
		wsprintf(item.pszText, _T("%d K"), plView->processInfo[i].pmc.PeakPagefileUsage / kb(1));
		ListView_SetItem(hListView, &item);
	}

	for (int i = plView->cProcess; i < cItems; i++)
		ListView_DeleteItem(hListView, i);
}

INT_PTR CALLBACK CreateDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	UNREFERENCED_PARAMETER(lParam);
	switch (message)
	{
	case WM_INITDIALOG:
		return (INT_PTR)TRUE;

	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		case IDOK:
			{
				BOOL bSuccessful = FALSE;

				HWND editControl = GetDlgItem(hDlg, IDC_EDITPATH);

				TCHAR szPath[MAX_PATH];
				ZeroMemory(szPath, sizeof(szPath));
				SendMessage(editControl, WM_GETTEXT, sizeof(szPath), (LPARAM)szPath);

				HWND checkBox = GetDlgItem(hDlg, IDC_CREATEASOTHERUSER);
				if (SendMessage(checkBox, BM_GETCHECK, 0, 0) == BST_CHECKED)
				{
					CREDUI_INFO cui;
					TCHAR szName[CREDUI_MAX_USERNAME_LENGTH+1];
					TCHAR szPwd[CREDUI_MAX_PASSWORD_LENGTH+1];
					BOOL fSave;
					DWORD dwErr;

					cui.cbSize = sizeof(CREDUI_INFO);
					cui.hwndParent = hWnd;

					cui.pszMessageText = _T("Introduzca la información de Login o de Usuario");
					cui.pszCaptionText = _T("TaskManagerLite");
					cui.hbmBanner = NULL;
					fSave = FALSE;
					SecureZeroMemory(szName, sizeof(szName));
					SecureZeroMemory(szPwd, sizeof(szPwd));
					dwErr = CredUIPromptForCredentials( 
						&cui,
						_T(""),
						NULL,
						0,
						szName,
						CREDUI_MAX_USERNAME_LENGTH+1,
						szPwd,
						CREDUI_MAX_PASSWORD_LENGTH+1,
						&fSave,
						CREDUI_FLAGS_GENERIC_CREDENTIALS | CREDUI_FLAGS_ALWAYS_SHOW_UI | CREDUI_FLAGS_DO_NOT_PERSIST);  

					if(!dwErr)
					{
						bSuccessful = ProcessCreator(szName, szPwd, szPath);

						SecureZeroMemory(szName, sizeof(szName));
						SecureZeroMemory(szPwd, sizeof(szPwd));
					}
				}
				else
				{
					bSuccessful = ProcessCreator(_T(""), _T(""), szPath);
				}
				EndDialog(hDlg, LOWORD(wParam));
				return bSuccessful;
			}

		case IDCANCEL:
			{
				EndDialog(hDlg, LOWORD(wParam));
				return FALSE;
				//break;
			}

		case IDC_SELECT_EXE:
			{
				OPENFILENAME ofn;
				TCHAR szFile[MAX_PATH];

				ZeroMemory(&ofn, sizeof(ofn));
				ofn.lStructSize = sizeof(ofn);
				ofn.hwndOwner = hWnd;
				ofn.lpstrFile = szFile;
				ofn.lpstrFile[0] = '\0';
				ofn.nMaxFile = sizeof(szFile);
				ofn.lpstrFilter = _T("Todos\0*.*\0Binarios\0*.exe\0");
				ofn.nFilterIndex = 1;
				ofn.lpstrFileTitle = NULL;
				ofn.nMaxFileTitle = 0;
				ofn.lpstrInitialDir = NULL;
				ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;

				if (!GetOpenFileName(&ofn)) 
					break;

				SendMessage (GetDlgItem(hDlg, IDC_EDITPATH), WM_SETTEXT, 0, (LPARAM) szFile);
			}
			break;

		default:
			break;
		}
		break;
	}
	return FALSE;
}

INT_PTR CALLBACK EndDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	UNREFERENCED_PARAMETER(lParam);
	switch (message)
	{
	case WM_INITDIALOG:
		return TRUE;

	case WM_COMMAND:
		if (LOWORD(wParam) == IDYES)
		{
			if (SendMessage(GetDlgItem(hDlg, IDC_CHECK), BM_GETCHECK, 0, 0) == BST_CHECKED)
				bShowWarningDlg = 0;
			EndDialog(hDlg, LOWORD(wParam));
			return TRUE;
		}
		else if (LOWORD (wParam) == IDNO)
		{
			EndDialog(hDlg, LOWORD(wParam));
			return FALSE;
		}
		break;
	}
	return FALSE;
}

INT_PTR CALLBACK AboutDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	PAINTSTRUCT ps;
	HDC hdc;
	RECT rect;

	HWND hLabel1, hLabel2;

	UNREFERENCED_PARAMETER(lParam);
	switch (message)
	{
	case WM_INITDIALOG:
		{
			RECT rcClient;
			GetWindowRect(hDlg, &rcClient);
		
			ExtendIntoClientBottom(hDlg);
		}
		return TRUE;

	case WM_LBUTTONUP:
		EndDialog(hDlg, LOWORD(wParam));
		return TRUE;

		break;


	case WM_PAINT:
		{
			hdc = BeginPaint(hDlg, &ps);
			GetClientRect(hDlg, &rect);
			FillRect(hdc, &rect, (HBRUSH)GetStockObject(BLACK_BRUSH));
			PaintCustomCaption(hDlg, hdc);
			EndPaint(hDlg, &ps);
		}

		break;

	case WM_COMMAND:
		if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
		{
			EndDialog(hDlg, LOWORD(wParam));
			return TRUE;
		}
		break;
	}
	return FALSE;
}

INT_PTR CALLBACK ModulesDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	static HWND hwndBtnOk;
	static HWND hwndBtnOk2;
	static HWND hwndListViewM = NULL;
	static HWND hwndShowDDOnly;

	static PModuleList pML;
	static PHeapList pHL;
	static PThreadList pTL;

	UNREFERENCED_PARAMETER(lParam);
	switch (message)
	{
	case WM_INITDIALOG:

		RECT rect;
		GetClientRect(hDlg, &rect);

		hwndBtnOk = GetDlgItem(hDlg, IDOK);
		hwndShowDDOnly = GetDlgItem(hDlg, IDC_SHOWDDONLY);

		MoveWindow(
			hwndBtnOk,
			rect.right - CBTN_MARGIN - CBTN_X,						// x coord
			rect.bottom - CBTN_MARGIN - CBTN_Y,						// y coord
			CBTN_X,													// cx
			CBTN_Y,													// cy
			TRUE);

		MoveWindow(
			hwndShowDDOnly,
			CBTN_MARGIN,
			rect.bottom - CBTN_MARGIN - CBTN_Y,
			CBTN_X * 3,
			CBTN_Y,
			TRUE);

		hwndListViewM = CreateWindow(
			WC_LISTVIEW,
			_T(""),
			WS_VISIBLE | WS_CHILD | LVS_REPORT | WS_BORDER | 
			LVS_SINGLESEL | LVS_SHOWSELALWAYS | WS_VSCROLL | WS_HSCROLL,
			CBTN_MARGIN,											// x coord
			CBTN_MARGIN,											// y coord
			rect.right - 2 * CBTN_MARGIN,							// cx
			rect.bottom - 5 * CBTN_MARGIN,							// cy
			hDlg,
			(HMENU) 4,
			hInst, NULL);

		ListView_SetExtendedListViewStyle(hwndListViewM,
			LVS_EX_DOUBLEBUFFER | LVS_EX_FULLROWSELECT);
		
		switch (ListWhat)
		{
		case Modules:
			{
				Button_SetText(hDlg, _T("Módulos"));
				
				pML = new ModuleList();
				ListProcessModules(hWnd, PID, pML);

				int nColumn = 0;
				LVCOLUMN column;
				column.mask = LVCF_WIDTH | LVCF_TEXT;
				column.pszText = _T("Name");
				column.cx = charXSize * 21;
				ListView_InsertColumn(hwndListViewM, nColumn++, &column);

				column.pszText = _T("Device Driver");
				column.cx = charXSize * 10;
				ListView_InsertColumn(hwndListViewM, nColumn++, &column);

				column.pszText = _T("Base Address");
				column.cx = charXSize * 10;
				ListView_InsertColumn(hwndListViewM, nColumn++, &column);

				column.pszText = _T("Base Size");
				column.cx = charXSize * 10;
				ListView_InsertColumn(hwndListViewM, nColumn++, &column);

				column.pszText = _T("Path");
				column.cx = charXSize * 30;
				ListView_InsertColumn(hwndListViewM, nColumn++, &column);

				column.pszText = _T("Global Ref");
				column.cx = charXSize * 10;
				ListView_InsertColumn(hwndListViewM, nColumn++, &column);

				column.pszText = _T("Process Ref");
				column.cx = charXSize * 10;
				ListView_InsertColumn(hwndListViewM, nColumn++, &column);

				TCHAR szBuffer[260];
				LVITEM item;
				ZeroMemory(&item, sizeof(item));
				item.cchTextMax = 260;
				item.mask = LVIF_TEXT;
				item.pszText = szBuffer;

				for (int i = 0; i < pML->cModule; i++)
				{
					int cSubItems = 0;
					item.iItem = i;

					item.iSubItem = cSubItems++;
					_tcscpy(item.pszText, pML->module[i].szModule);
					ListView_InsertItem(hwndListViewM, &item);

					item.iSubItem = cSubItems++;
					_tcscpy(item.pszText, pML->IsDeviceDriver[i]? _T("Sí") : _T("No"));
					ListView_SetItem(hwndListViewM, &item);

					item.iSubItem = cSubItems++;
					wsprintf(item.pszText, _T("0x%08x"), pML->module[i].modBaseAddr);
					ListView_SetItem(hwndListViewM, &item);

					item.iSubItem = cSubItems++;
					wsprintf(item.pszText, _T("%d K"), pML->module[i].modBaseSize / kb(1));
					ListView_SetItem(hwndListViewM, &item);

					item.iSubItem = cSubItems++;
					_tcscpy(item.pszText, pML->module[i].szExePath);
					ListView_SetItem(hwndListViewM, &item);

					item.iSubItem = cSubItems++;
					wsprintf(item.pszText, _T("%d"), pML->module[i].GlblcntUsage);
					ListView_SetItem(hwndListViewM, &item);

					item.iSubItem = cSubItems++;
					wsprintf(item.pszText, _T("%d"), pML->module[i].ProccntUsage);
					ListView_SetItem(hwndListViewM, &item);
				}
			}
			break;

		case Heaps:
			{				
				Button_SetText(hDlg, _T("Heaps"));
				ShowWindow(hwndShowDDOnly, SW_HIDE);
				
				pHL = new HeapList();
				ListProcessHeaps(hWnd, PID, pHL);
				
				int nColumn = 0;				
				LVCOLUMN column;
				column.mask = LVCF_WIDTH | LVCF_TEXT;
				column.pszText = _T("Heap ID");
				column.cx = charXSize * 10;
				ListView_InsertColumn(hwndListViewM, nColumn++, &column);

				column.pszText = _T("Address");
				column.cx = charXSize * 10;
				ListView_InsertColumn(hwndListViewM, nColumn++, &column);

				column.pszText = _T("Block Size");
				column.cx = charXSize * 10;
				ListView_InsertColumn(hwndListViewM, nColumn++, &column);

				column.pszText = _T("Locks");
				column.cx = charXSize * 10;
				ListView_InsertColumn(hwndListViewM, nColumn++, &column);

				TCHAR szBuffer[260];
				LVITEM item;
				ZeroMemory(&item, sizeof(item));
				item.cchTextMax = 260;
				item.mask = LVIF_TEXT;
				item.pszText = szBuffer;

				for (int i = 0; i < pHL->cHeap; i++)
				{				
					int cSubItems = 0;
					item.iItem = i;

					item.iSubItem = cSubItems++;
					wsprintf(item.pszText, _T("%d"), pHL->heap[i].th32HeapID);
					ListView_InsertItem(hwndListViewM, &item);

					item.iSubItem = cSubItems++;
					wsprintf(item.pszText, _T("0x%08x"), pHL->heap[i].dwAddress);
					ListView_SetItem(hwndListViewM, &item);

					item.iSubItem = cSubItems++;
					wsprintf(item.pszText, _T("%d B"), pHL->heap[i].dwBlockSize);
					ListView_SetItem(hwndListViewM, &item);

					item.iSubItem = cSubItems++;
					wsprintf(item.pszText, _T("%d"), pHL->heap[i].dwLockCount);
					ListView_SetItem(hwndListViewM, &item);
				}
			}
			break;

		case Threads:
			{
				Button_SetText(hDlg, _T("Hilos"));
				ShowWindow(hwndShowDDOnly, SW_HIDE);
				
				pTL = new ThreadList();
				ListProcessThreads(hWnd, PID, pTL);

				int nColumn = 0;
				LVCOLUMN column;
				column.mask = LVCF_WIDTH | LVCF_TEXT;
				column.pszText = _T("Thread ID");
				column.cx = charXSize * 10;
				ListView_InsertColumn(hwndListViewM, nColumn++, &column);

				column.pszText = _T("Base Priority");
				column.cx = charXSize * 10;
				ListView_InsertColumn(hwndListViewM, nColumn++, &column);

				column.pszText = _T("Delta Priority");
				column.cx = charXSize * 10;
				ListView_InsertColumn(hwndListViewM, nColumn++, &column);

				TCHAR szBuffer[260];
				LVITEM item;
				ZeroMemory(&item, sizeof(item));
				item.cchTextMax = 260;
				item.mask = LVIF_TEXT;
				item.pszText = szBuffer;

				for (int i = 0; i < pTL->cThread; i++)
				{				
					int cSubItems = 0;
					item.iItem = i;

					item.iSubItem = cSubItems++;
					wsprintf(item.pszText, _T("%d"), pTL->thread[i].th32ThreadID);
					ListView_InsertItem(hwndListViewM, &item);

					item.iSubItem = cSubItems++;
					wsprintf(item.pszText, _T("%d"), pTL->thread[i].tpBasePri);
					ListView_SetItem(hwndListViewM, &item);

					item.iSubItem = cSubItems++;
					wsprintf(item.pszText, _T("%d"), pTL->thread[i].tpDeltaPri);
					ListView_SetItem(hwndListViewM, &item);
				}
			}
			break;
		}

		UpdateWindow(hDlg);

		return TRUE;

	case WM_SIZE:
		{
			DWORD dwNewWidth = LOWORD(lParam);
			DWORD dwNewHeight = HIWORD(lParam);

			MoveWindow(
				hwndBtnOk,
				dwNewWidth - CBTN_MARGIN - CBTN_X,						// x coord
				dwNewHeight - CBTN_MARGIN - CBTN_Y,						// y coord
				CBTN_X,													// cx
				CBTN_Y,													// cy
				TRUE);

			MoveWindow(
				hwndListViewM,
				CBTN_MARGIN,											// x coord
				CBTN_MARGIN,											// y coord
				dwNewWidth - 2 * CBTN_MARGIN,							// cx
				dwNewHeight - 5 * CBTN_MARGIN,							// cy
				TRUE);

			MoveWindow(
				hwndShowDDOnly,
				CBTN_MARGIN,
				dwNewHeight - CBTN_MARGIN - CBTN_Y,
				CBTN_X * 3,
				CBTN_Y,
				TRUE);

		}
		break;

	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		case IDOK:
		case IDCANCEL:
			switch (ListWhat)
			{
			case Modules:
				FreeListProcessModules(pML);
				break;
			case Heaps:
				FreeListProcessHeaps(pHL);
				break;
			case Threads:
				FreeListProcessThreads(pTL);
				break;
			}

			EndDialog(hDlg, LOWORD(wParam));

			return TRUE;
			break;

		case IDC_SHOWDDONLY:
			{
				BOOL bChecked = Button_GetCheck(hwndShowDDOnly);

				TCHAR szBuffer[260];
				LVITEM item;
				ZeroMemory(&item, sizeof(item));
				item.cchTextMax = 260;
				item.mask = LVIF_TEXT;
				item.pszText = szBuffer;
				
				ListView_DeleteAllItems(hwndListViewM);

				int j = 0;
				for (int i = 0; i < pML->cModule; i++)
				{
					if (bChecked && !pML->IsDeviceDriver[i])
						continue;
					else
						j++;

					int cSubItems = 0;
					item.iItem = j - 1;

					item.iSubItem = cSubItems++;
					_tcscpy(item.pszText, pML->module[i].szModule);
					ListView_InsertItem(hwndListViewM, &item);

					item.iSubItem = cSubItems++;
					_tcscpy(item.pszText, pML->IsDeviceDriver[i]? _T("Sí") : _T("No"));
					ListView_SetItem(hwndListViewM, &item);

					item.iSubItem = cSubItems++;
					wsprintf(item.pszText, _T("0x%08x"), pML->module[i].modBaseAddr);
					ListView_SetItem(hwndListViewM, &item);

					item.iSubItem = cSubItems++;
					wsprintf(item.pszText, _T("%d K"), pML->module[i].modBaseSize / kb(1));
					ListView_SetItem(hwndListViewM, &item);

					item.iSubItem = cSubItems++;
					_tcscpy(item.pszText, pML->module[i].szExePath);
					ListView_SetItem(hwndListViewM, &item);

					item.iSubItem = cSubItems++;
					wsprintf(item.pszText, _T("%d"), pML->module[i].GlblcntUsage);
					ListView_SetItem(hwndListViewM, &item);

					item.iSubItem = cSubItems++;
					wsprintf(item.pszText, _T("%d"), pML->module[i].ProccntUsage);
					ListView_SetItem(hwndListViewM, &item);
				}
			}
			break;
		}
		break;
	}
	return FALSE;
}

VOID ProcessTerminator(HANDLE hProcessInstance)
{ 
	DWORD lpExitCode = 0;
	UINT uExit = 0;
	GetExitCodeProcess(hProcessInstance, &lpExitCode);
	uExit = (UINT)(lpExitCode);
	TerminateProcess(hProcessInstance, uExit);
}

BOOL ProcessCreator(TCHAR *username, TCHAR *password, TCHAR *tcmd)
{
	HANDLE hToken;
	DWORD dwSize;
	LPVOID lpvEnv;
	STARTUPINFOW si;
	PROCESS_INFORMATION pi;
	ZeroMemory(&si, sizeof(si));
	si.cb = sizeof(si);  
	ZeroMemory(&pi, sizeof(pi));
	WCHAR szUserProfile[CRED_MAX_USERNAME_LENGTH+1] = L"";

	if (strlen(tcmd) == 0)
		return FALSE;

	WCHAR wUsername[kb(1)];
	WCHAR wPassword[kb(1)];
	WCHAR wTcmd[kb(1)];

	mbstowcs(wUsername, username, kb(1));
	mbstowcs(wPassword, password, kb(1));
	mbstowcs(wTcmd, tcmd, kb(1));

	if (wcslen(wUsername) == 0 && wcslen(wPassword) == 0)
	{
		if(!CreateProcessW(NULL, wTcmd, NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi))
		{
			printError(hWnd, _T("CreateProcess"));
			return FALSE;
		}
	}
	else 
	{
		if(!LogonUserW(wUsername, NULL, wPassword, LOGON32_LOGON_INTERACTIVE, LOGON32_PROVIDER_DEFAULT, &hToken))
		{
			printError(hWnd, _T("LogonUser"));
			return FALSE;
		}
		if (!CreateEnvironmentBlock(&lpvEnv, hToken, TRUE))
		{
			printError(hWnd, _T("CreateEnvironmentBlock"));
			return FALSE;
		}

		dwSize = sizeof(szUserProfile)/sizeof(TCHAR);

		if (!GetUserProfileDirectoryW(hToken, szUserProfile, &dwSize))
		{
			printError(hWnd, _T("GetUserProfileDirectory"));
			return FALSE;
		}
		if (!CreateProcessWithLogonW(wUsername, NULL, wPassword, LOGON_WITH_PROFILE,
			NULL, wTcmd, CREATE_UNICODE_ENVIRONMENT, lpvEnv, szUserProfile, &si, &pi))
		{
			printError(hWnd, _T("CreateProcessWithLogonW"));
			return FALSE;
		}
		if (!DestroyEnvironmentBlock(lpvEnv))
		{
			printError(hWnd, _T("DestroyEnvironmentBlock"));
			return FALSE;
		}
	}
	return TRUE;
}