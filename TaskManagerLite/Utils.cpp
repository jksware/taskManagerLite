#include "stdafx.h"
#include "Utils.h"
#include "resource.h"

INT GetSelectionPID(HWND hwnd)
{
	int index = ListView_GetSelectionMark(hwnd);
	if (index == -1)
		return index;

	LVITEM item;
	ZeroMemory(&item, sizeof(item));
	TCHAR szBuffer[10];
	ZeroMemory(szBuffer, sizeof(szBuffer));

	item.cchTextMax = sizeof(szBuffer);
	item.mask = LVIF_TEXT;
	item.iItem = index;
	item.iSubItem = 1;
	item.pszText = szBuffer;
	ListView_GetItem(hwnd, &item);
	return _ttoi64(item.pszText);
}

VOID PaintCustomCaption(HWND hWnd, HDC hdc)
{
	RECT rcClient;
	GetClientRect(hWnd, &rcClient);

	HTHEME hTheme = OpenThemeData(NULL, L"CompositedWindow::Window");
	if (hTheme)
	{
		HDC hdcPaint = CreateCompatibleDC(hdc);
		if (hdcPaint)
		{
			int cx = rcClient.right - rcClient.left;
			int cy = rcClient.bottom - rcClient.top;

			BITMAPINFO dib = { 0 };
			dib.bmiHeader.biSize            = sizeof(BITMAPINFOHEADER);
			dib.bmiHeader.biWidth           = cx;
			dib.bmiHeader.biHeight          = -cy;
			dib.bmiHeader.biPlanes          = 1;
			dib.bmiHeader.biBitCount        = 32;
			dib.bmiHeader.biCompression     = BI_RGB;

			HBITMAP hbm = CreateDIBSection(hdc, &dib, DIB_RGB_COLORS, NULL, NULL, 0);
			if (hbm)
			{
				HBITMAP hbmOld = (HBITMAP)SelectObject(hdcPaint, hbm);

				DTTOPTS DttOpts = {sizeof(DTTOPTS)};
				DttOpts.dwFlags = DTT_COMPOSITED | DTT_GLOWSIZE | DTT_TEXTCOLOR;

				LOGFONT lgFont;
				HFONT hFontOld = NULL;
				if (FALSE)
				{
					HFONT hFont = CreateFontIndirect(&lgFont);
					hFontOld = (HFONT) SelectObject(hdcPaint, hFont);
				}
				else
				{
					hFontOld = (HFONT)SelectObject(hdcPaint, GetMessageFont());
				}

				RECT rcPaint = rcClient;
				rcPaint.top += CBTN_MARGIN;
				rcPaint.right -= CBTN_MARGIN;
				rcPaint.left += CBTN_MARGIN;
				rcPaint.bottom -= CBTN_MARGIN;

				WCHAR szBuffer[kb(1)];
				GetWindowTextW(GetDlgItem(hWnd, IDC_STATIC3), szBuffer, kb(1));

				DttOpts.iGlowSize = 12;
				DttOpts.crText = 0x11111111;
				DrawThemeTextEx(hTheme, 
					hdcPaint, 
					0, 0, 
					szBuffer, 
					-1, 
					DT_EXPANDTABS | DT_LEFT | DT_WORD_ELLIPSIS, 
					&rcPaint, 
					&DttOpts);

				rcPaint.top += 20;

				wsprintfW(szBuffer, L"Compilado con MSVC-%d en %S a las %S.", _MSC_VER, __DATE__, __TIME__);
				DrawThemeTextEx(hTheme, 
					hdcPaint, 
					0, 0, 
					szBuffer, 
					-1, 
					DT_EXPANDTABS | DT_LEFT | DT_WORD_ELLIPSIS, 
					&rcPaint, 
					&DttOpts);

				rcPaint.top += 20;

				GetWindowTextW(GetDlgItem(hWnd, IDC_STATIC2), szBuffer, kb(1));
				DttOpts.iGlowSize = 10;
				DttOpts.crText = 0xff774400;				
				DrawThemeTextEx(hTheme, 
					hdcPaint, 
					0, 0, 
					szBuffer, 
					-1, 
					DT_EXPANDTABS | DT_LEFT | DT_WORD_ELLIPSIS, 
					&rcPaint, 
					&DttOpts);

				rcPaint.top += 180;

				DttOpts.crText = 0x11111111;				
				DrawThemeTextEx(hTheme, 
					hdcPaint, 
					0, 0, 
					L"Presione <ESC> o haga clic para cerrar.", 
					-1, 
					DT_EXPANDTABS | DT_LEFT | DT_WORD_ELLIPSIS, 
					&rcPaint, 
					&DttOpts);

				BitBlt(hdc, 0, 0, cx, cy, hdcPaint, 0, 0, SRCCOPY);

				SelectObject(hdcPaint, hbmOld);
				if (hFontOld)
				{
					SelectObject(hdcPaint, hFontOld);
				}
				DeleteObject(hbm);
			}
			DeleteDC(hdcPaint);
		}
		CloseThemeData(hTheme);
	}
}

BOOL CALLBACK SetChildFont(HWND hwndChild, LPARAM lParam)
{
	SendMessage(hwndChild, WM_SETFONT, (WPARAM)(HFONT)lParam, TRUE);
	return TRUE;
}

HRESULT ExtendIntoClientBottom(HWND hwnd)
{
	MARGINS margins = {-1};
	HRESULT hr = S_OK;

	hr = DwmExtendFrameIntoClientArea(hwnd, &margins);
	return hr;
}

HRESULT EnableBlurBehind(HWND hwnd)
{
	HRESULT hr = S_OK;

	DWM_BLURBEHIND bb = {0};

	bb.dwFlags = DWM_BB_ENABLE;
	bb.fEnable = TRUE;
	bb.hRgnBlur = NULL;

	hr = DwmEnableBlurBehindWindow(hwnd, &bb);
	return hr;
}

HFONT GetMessageFont()
{
	LOGFONT lf = {0};
	NONCLIENTMETRICS ncm = { sizeof(NONCLIENTMETRICS) };
	SystemParametersInfo (
		SPI_GETNONCLIENTMETRICS, sizeof(NONCLIENTMETRICS),
		&ncm, false );
	lf = ncm.lfMessageFont;
	return CreateFontIndirect ( &lf );
}

HFONT GetCaptionFont()
{
	LOGFONT lf = {0};
	NONCLIENTMETRICS ncm = { sizeof(NONCLIENTMETRICS) };
	SystemParametersInfo (
		SPI_GETNONCLIENTMETRICS, sizeof(NONCLIENTMETRICS),
		&ncm, false );
	lf = ncm.lfCaptionFont;
	return CreateFontIndirect ( &lf );
}

HFONT GetMenuFont()
{
	LOGFONT lf = {0};
	NONCLIENTMETRICS ncm = { sizeof(NONCLIENTMETRICS) };
	SystemParametersInfo (
		SPI_GETNONCLIENTMETRICS, sizeof(NONCLIENTMETRICS),
		&ncm, false );
	lf = ncm.lfMenuFont;
	return CreateFontIndirect ( &lf );
}

HFONT GetSmCaptionFont()
{
	LOGFONT lf = {0};
	NONCLIENTMETRICS ncm = { sizeof(NONCLIENTMETRICS) };
	SystemParametersInfo (
		SPI_GETNONCLIENTMETRICS, sizeof(NONCLIENTMETRICS),
		&ncm, false );
	lf = ncm.lfSmCaptionFont;
	return CreateFontIndirect ( &lf );
}


HFONT GetStatusFont()
{
	LOGFONT lf = {0};
	NONCLIENTMETRICS ncm = { sizeof(NONCLIENTMETRICS) };
	SystemParametersInfo (
		SPI_GETNONCLIENTMETRICS, sizeof(NONCLIENTMETRICS),
		&ncm, false );
	lf = ncm.lfStatusFont;
	return CreateFontIndirect ( &lf );
}