#define _CRT_SECURE_NO_WARNINGS
#include <dshow.h> 
#include <commctrl.h> 
#include <commdlg.h> 
#include <stdio.h>
#include <tchar.h>
#include <atlbase.h>

#include "ticker.h"
#include "bitmap.h" 
#include "vmrutil.h"
#include <vmr9.h>
#include <cstdarg>

HINSTANCE ghInst = 0;
HWND	ghApp = 0;
HMENU	ghMenu = 0;
TCHAR	g_szFileName[MAX_PATH] = { 0 };
DWORD	g_dwGraphRegister = 0;
RECT	g_rcDest = { 0 };

HRESULT PlayMovieInWindow(LPTSTR szFile)
{
	USES_CONVERSION;
	WCHAR wFile[MAX_PATH];
	HRESULT hr;

	if (szFile == NULL)
		return E_POINTER;

	JIF(CoCreateInstance(CLSID_FilterGraph, NULL, CLSCTX_INPROC_SERVER, IID_IGraphBuilder, (void**)&pGB));

	if (SUCCEEDED(hr)) {
		CComPtr <IBaseFilter> pVmr;
		if (FAILED(hr = RenderFileToVMR9(pGB, wFile, pVmr, TRUE)))
			return hr;
		JIF(pGB->QueryInterface(IID_IMediaControl, (void**)&pMC));
		JIF(pGB->QueryInterface(IID_IMediaEventEx, (void**)&pME));
		JIF(pGB->QueryInterface(IID_IMediaSeeking, (void**)&pMS));

		if (CheckVideoVisibility()) {
			JIF(InitVideoWindow(1, 1));
		}
		else
		{
			Msg(TEXT("This sample requires media with a video component. ")
				TEXT("Please select another file."));
			return E_FAIL;
		}

		JIF(pME->SetNotifyWindow((OAHWND)ghApp, WM_GRAPHNOTIFY, 0));
		if (g_dwTickerFlags & MARK_STATIC_IMAGE)
		{
			hr = BlendApplicationImage(ghApp);
			if (FAILED(hr))
				PostMessage(ghApp, WM_CLOSE, 0, 0);
			CheckMenuItem(ghMenu, ID_TICKER_STATIC_IMAGE, MF_CHECKED);
			CheckMenuItem(ghMenu, ID_TICKER_DYNAMIC_TEXT, MF_UNCHECKED);
		}
		else
		{
			if (!g_hFont)
				g_hFont = SetTextFont(FALSE);

			hr = BlendApplicationText(ghApp, g_szAppText);
			if (FAILED(hr))
				PostMessage(ghApp, WM_CLOSE, 0, 0);
			CheckMenuItem(ghMenu, ID_TICKER_STATIC_IMAGE, MF_UNCHECKED);
			CheckMenuItem(ghMenu, ID_TICKER_DYNAMIC_TEXT, MF_CHECKED);
		}

		ShowWindow(ghApp, SW_SHOWNORMAL);
		UpdateWindow(ghApp);
		SetForegroundWindow(ghApp);
		SetFocus(ghApp);

		JIF(pMC->Run());

		PostMessage(ghApp, WM_COMMAND, ID_SLIDE, 0);
	}
	return hr;
}

HRESULT InitVideoWindow(int nMultiplier, int nDivider)
{
	LONG lHeight, lWidth;
	HRESULT hr = S_OK;
	if (!pWC) return S_OK;
	
	hr = pWC->GetNativeVideoSize(&lWidth, &lHeight, NULL, NULL); 
	if (hr == E_NOINTERFACE) return S_OK;
	
	lWidth = lWidth * nMultiplier / nDivider;
	lHeight = lHeight * nMultiplier / nDivider;

	int nTitleHeight = GetSystemMetrics(SM_CYCAPTION);
	int nBorderWidth = GetSystemMetrics(SM_CXBORDER); 
	int nBorderHeight = GetSystemMetrics(SM_CYBORDER);
	
	SetWindowPos(ghApp, NULL, 0, 0, lWidth + 2*nBorderWidth, lHeight+nTitleHeight + 2*nBorderHeight, SWP_NOMOVE | SWP_NOOWNERZORDER);
	GetClientRect(ghApp, &g_rcDest);
	hr = pWC->SetVideoPosition(NULL, &g_rcDest);
	if (FAILED(hr)) 
		Msg(TEXT("SetVideoPosition FAILED! hr=0x%xWrWn"), hr);

	return hr;
}

HRESULT InitPlayerWindow(void)
{
	// Reset to a default size for audio and after closing a clip 
	SetWindowPos(ghApp, NULL, 0, 0, DEFAULT_PLAYER_WIDTH, DEFAULT_PLAYER_HEIGHT, SWP_NOMOVE | SWP_NOOWNERZORDER);
	return S_OK;
}

void MoveVideoWindow(void)
{
	HRESULT hr;
	// Track the movement of the container window and resize as needed 
	if (pWC)
	{
		GetClientRect(ghApp, &g_rcDest);
		hr = pWC->SetVideoPosition(NULL, &g_rcDest);
		if (FAILED(hr))
			Msg(TEXT("SetVideoPosition FAILED! 	hr = 0x%xWrWn"), hr);
	}
}

BOOL CheckVideoVisibility()
{
	HRESULT hr;
	LONG lWidth = 0, lHeight = 0;

	if (!pWC)
	{
		return FALSE;
	}

	hr = pWC->GetNativeVideoSize(&lWidth, &lHeight, 0, 0);
	if (FAILED(hr))
	{
		return FALSE;
	}

	if ((lWidth == 0) && (lHeight == 0))
		return FALSE;

	return TRUE;
}

void OpenClip()
{
	HRESULT hr;

	if (g_szFileName[0] == '0')
	{
		TCHAR szFilename[MAX_PATH];

		InitPlayerWindow();
		SetForegroundWindow(ghApp);

		if (!GetClipFileName(szFilename)) {
			DWORD dwDlgErr = CommDlgExtendedError();

			if (dwDlgErr)
				Msg(TEXT("GetClipFileName Failed! Error=0x%xWrWn"), GetLastError());
			return;
		}

		if (_tcsstr((_tcslwr(szFilename)), TEXT(".asx")))
		{
			Msg(TEXT("ASX Playlists are not supported by this application.WnWn")
				TEXT("Please select a valid media file.#"));
			return;
		}
		lstrcpyn(g_szFileName, szFilename, NUMELMS(g_szFileName));
	}
	EnableTickerMenu(TRUE);

	hr = PlayMovieInWindow(g_szFileName);

	if (FAILED(hr)) CloseClip();
}

BOOL GetClipFileName(LPTSTR szName)
{
	static OPENFILENAME ofn = { 0 };
	static BOOL bSetInitialDir = FALSE;

	*szName = 0;

	ofn.lStructSize = sizeof(OPENFILENAME);
	ofn.hwndOwner = ghApp;
	ofn.lpstrFilter = NULL;
	ofn.lpstrFilter = FILE_FILTER_TEXT;
	ofn.lpstrCustomFilter = NULL;
	ofn.nFilterIndex = 1;
	ofn.lpstrFile = szName;
	ofn.nMaxFile = MAX_PATH;
	ofn.lpstrTitle = TEXT("Open Image File...");
	ofn.lpstrFileTitle = NULL;
	ofn.lpstrDefExt = TEXT("0");
	ofn.Flags = OFN_FILEMUSTEXIST | OFN_READONLY | OFN_PATHMUSTEXIST;


	// Remember the path of the first selected file
	if (bSetInitialDir == FALSE)
	{
		ofn.lpstrInitialDir = DEFAULT_MEDIA_PATH; 
		bSetInitialDir = TRUE;
	}
	else
		ofn.lpstrInitialDir = NULL;
	
	return GetOpenFileName((LPOPENFILENAME)&ofn);
}
void CloseClip()
{
	HRESULT hr;

	if (pMC)
		hr = pMC->Stop();

	CloseInterfaces();

	g_szFileName[0] = L'0';

	RECT rect;
	GetClientRect(ghApp, &rect);
	InvalidateRect(ghApp, &rect, TRUE);

	InitPlayerWindow();
	EnableTickerMenu(FALSE);
}
void CloseInterfaces()
{
	// Clear ticker state and timer settings ClearTickerState();
	// Release and zero DirectShow interfaces
	SAFE_RELEASE(pME);
	SAFE_RELEASE(pMS);
	SAFE_RELEASE(pMC);
	SAFE_RELEASE(pWC);
	SAFE_RELEASE(pBMP);
	SAFE_RELEASE(pGB);
}


HRESULT HandleGraphEvent()
{
	LONG evCode, evParam1, evParam2;
	HRESULT hr = S_OK;

	if (!pME) return S_OK;

	while (SUCCEEDED(pME->GetEvent(&evCode, (LONG_PTR*)&evParam1,
		(LONG_PTR*)&evParam2, 0)))
	{
		hr = pME->FreeEventParams(evCode, evParam1, evParam2);
		if (EC_COMPLETE == evCode) {
			LONGLONG pos = 0;
			// Reset to first frame of movie
			hr = pMS->SetPositions(&pos, AM_SEEKING_AbsolutePositioning, NULL, AM_SEEKING_NoPositioning);
			if (FAILED(hr)) {
				hr = pMC->Stop();
				hr = pMC->Run();
			}
		}
	}
	return hr;
}

void Msg(TCHAR* szFormat, ...)
{
	TCHAR szBuffer[1024]; // Large buffer for long filenames or URLS 
	const size_t NUMCHARS = sizeof(szBuffer) / sizeof(szBuffer[0]);
	const int LASTCHAR = NUMCHARS - 1;

	va_list pargs;
	va_start(pargs, szFormat);


	_vsntprintf(szBuffer, NUMCHARS - 1, szFormat, pargs);
	va_end(pargs);

	MessageBox(NULL, szBuffer, TEXT("VMRTicker Sample"), MB_OK);
}
LRESULT CALLBACK AboutDlgProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case WM_INITDIALOG: return TRUE;
	case WM_COMMAND:
		if (wParam == IDOK) {
			EndDialog(hWnd, TRUE);
			return TRUE;
		}
		break;
	}
	return FALSE;
}

LRESULT CALLBACK AboutDlgProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case WM_INITDIALOG: return TRUE;
	case WM_COMMAND:
		if (wParam == IDOK)
		{
			EndDialog(hWnd, TRUE);
			return TRUE;
		}
		break;
	}
	return FALSE;
}
LRESULT CALLBACK TextD1gProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	static TCHAR szSaveText[DYNAMIC_TEXT_SIZE] = { 0 };
	switch (message)
	{
	case WM_INITDIALOG:
		_tcsncpy(szSaveText, g_szAppText, NUMELMS(szSaveText));
		SendMessage(GetDlgItem(hWnd, IDC_EDIT_TEXT), EM_LIMITTEXT, DYNAMIC_TEXT_SIZE, 0L);
		SetWindowText(GetDlgItem(hWnd, IDC_EDIT_TEXT), g_szAppText);
		return TRUE;
	case WM_COMMAND:
		switch (wParam)
		{
		case IDOK:
		{
			TCHAR szText[DYNAMIC_TEXT_SIZE];
			GetWindowText(GetDlgItem(hWnd, IDC_EDIT_TEXT), szText, DYNAMIC_TEXT_SIZE);
			_tcsncpy(g_szAppText, szText, NUMELMS(g_szAppText) - 1); BlendApplicationText(ghApp, g_szAppText);
			EndDialog(hWnd, TRUE);
			return TRUE;
		}
		break;
		case IDCANCEL:
			_tcsncpy(g_szAppText, szSaveText, NUMELMS(g_szAppText) - 1);
			BlendApplicationText(ghApp, g_szAppText);
			EndDialog(hWnd, TRUE);
			break;
		case IDC_SET_FONT:
		{
			TCHAR szTempText[DYNAMIC_TEXT_SIZE] = { 0 };

			g_hFont = UserSelectFont();

			GetWindowText(GetDlgItem(hWnd, IDC_EDIT_TEXT), szTempText, DYNAMIC_TEXT_SIZE);
			BlendApplicationText(ghApp, szTempText);
		}
		break;
		}
		break;
	}
	return FALSE;
}

LRESULT CALLBACK WndMainProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case WM_PAINT: OnPaint(hWnd);
		break;
	case WM_DISPLAYCHANGE:
		if (pWC)
			pWC->DisplayModeChanged();
		break;

	case WM_MOVE:
	case WM_SIZE:
		if (hWnd == ghApp)
			MoveVideoWindow();
		break;
	case WM_COMMAND:
		switch (wParam) {
		case ID_FILE_OPENCLIP:
			CloseClip();
			OpenClip();
			break;
		case ID_FILE_INITCLIP:
			OpenClip();
			break;
		case ID_FILE_EXIT:
			CloseClip();
			PostQuitMessage(0);
			break;
		case ID_FILE_CLOSE:
			CloseClip();
			break;
		case ID_DISABLE:
			FlipFlag(MARK_DISABLE);
			DisableTicker(g_dwTickerFlags);
			break;
		case ID_SLIDE:
			FlipFlag(MARK_SLIDE);
			SlideTicker(g_dwTickerFlags);
			break;

		case ID_TICKER_STATIC_IMAGE:
			g_dwTickerFlags |= MARK_STATIC_IMAGE;
			g_dwTickerFlags &= ~(MARK_DYNAMIC_TEXT);
			BlendApplicationImage(ghApp);
			CheckMenuItem(ghMenu, ID_TICKER_STATIC_IMAGE, MF_CHECKED);
			CheckMenuItem(ghMenu, ID_TICKER_DYNAMIC_TEXT, MF_UNCHECKED);
			break;
		case ID_TICKER_DYNAMIC_TEXT:
			g_dwTickerFlags |= MARK_DYNAMIC_TEXT;
			g_dwTickerFlags &= ~(MARK_STATIC_IMAGE);
			BlendApplicationText(ghApp, g_szAppText);
			CheckMenuItem(ghMenu, ID_TICKER_STATIC_IMAGE, MF_UNCHECKED);
			CheckMenuItem(ghMenu, ID_TICKER_DYNAMIC_TEXT, MF_CHECKED);
			break;
		case ID_SET_FONT:
			g_hFont = UserSelectFont();
			PostMessage(ghApp, WM_COMMAND, ID_TICKER_DYNAMIC_TEXT, 0);
			break;
		case ID_SET_TEXT:
			DialogBox(ghInst, MAKEINTRESOURCE(IDD_DIALOG_TEXT), ghApp, (DLGPROC)TextD1gProc);
			break;
		case ID_HELP_ABOUT:
			DialogBox(ghInst, MAKEINTRESOURCE(IDD_HELP_ABOUT),
				ghApp, (DLGPROC)AboutDlgProc);
			break;
		}
		break;
	case WM_GRAPHNOTIFY: HandleGraphEvent();
		break;
	case WM_CLOSE: SendMessage(ghApp, WM_COMMAND, ID_FILE_EXIT, 0);
		break;
	case WM_DESTROY: PostQuitMessage(0);
		break;
	default: return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return DefWindowProc(hWnd, message, wParam, lParam);
}

void OnPaint(HWND hwnd)
{
	HRESULT hr;
	PAINTSTRUCT ps;
	HDC	hdc;
	RECT rcClient;
		
	GetClientRect(hwnd, &rcClient);
	hdc = BeginPaint(hwnd, &ps);

	if (pWC) hr = pWC->RepaintVideo(hwnd, hdc);
	else FillRect(hdc, &rcClient, (HBRUSH)(COLOR_BTNFACE + 1));
	
	EndPaint(hwnd, &ps);
}

HRESULT InitializeWindowlessVMR(IBaseFilter** ppVmr9)
{
	IBaseFilter* pVmr = NULL;
	if (!ppVmr9) return E_POINTER;
	*ppVmr9 = NULL;
	// Create the Vmr and add it to the filter graph.
	HRESULT hr = CoCreateInstance(CLSID_VideoMixingRenderer9, NULL, CLSCTX_INPROC, IID_IBaseFilter, (void**)&pVmr);
	if (SUCCEEDED(hr)) {
		hr = pGB->AddFilter(pVmr, L"Video Mixing Renderer 9");
		if (SUCCEEDED(hr)) {
			CComPtr <IVMRFilterConfig9> pConfig;
			JIF(pVmr->QueryInterface(IID_IVMRFilterConfig9, (void**)&pConfig));
			JIF(pConfig->SetRenderingMode(VMR9Mode_Windowless));


			hr = pVmr->QueryInterface(IID_IVMRWindowlessControl9, (void**)&pWC);
			if (SUCCEEDED(hr)) {
				hr = pWC->SetVideoClippingWindow(ghApp);
				hr = pWC->SetBorderColor(RGB(0, 0, 0));
			}

#ifndef BILINEAR_FILTERING
			IVMRMixerControl9* pMix;
			hr = pVmr->QueryInterface(IID_IVMRMixerControl9, (void**)&pMix);
			if (SUCCEEDED(hr)) {
				DWORD dwPrefs = 0;
				hr = pMix->GetMixingPrefs(&dwPrefs);
				if (SUCCEEDED(hr)) {
					dwPrefs |= MixerPref_PointFiltering;
					dwPrefs &= ~(MixerPref_BiLinearFiltering);

					hr = pMix->SetMixingPrefs(dwPrefs);
				}
				pMix->Release();
			}
#endif
			hr = pVmr->QueryInterface(IID_IVMRMixerBitmap9, (void**)&pBMP);
		}
		else Msg(TEXT("Failed to add Vmr to graph! hr=0x%x\r\n"), hr);
		*ppVmr9 = pVmr;
	}
	else Msg(TEXT("Failed to create Vmr? hr=0x%xWrWn"), hr);
	return hr;
}


int PASCAL WinMain(HINSTANCE hInstC, HINSTANCE hInstP, LPSTR lpCmdLine, int nCmdShow)
{
	MSG msg = { 0 };
	WNDCLASS wc;
	USES_CONVERSION;

	if (FAILED(CoInitializeEx(NULL, COINIT_APARTMENTTHREADED))) {
		Msg(TEXT("CoInitialize Failed!\r\n"));
		return FALSE;
	}

	if (!VerifyVMR9()) return FALSE;

	if (lpCmdLine[0] = '0') {
		USES_CONVERSION;
		_tcsncpy(g_szFileName, A2T(lpCmdLine), NUMELMS(g_szFileName));
	}
	// Register the window class 
	ZeroMemory(&wc, sizeof wc);
	ghInst = wc.hInstance = hInstC;
	wc.lpfnWndProc = WndMainProc;
	wc.lpszClassName = CLASSNAME;
	wc.lpszMenuName = MAKEINTRESOURCE(IDR_MENU);
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	wc.hIcon = LoadIcon(hInstC, MAKEINTRESOURCE(IDI_TICKER));

	if (!RegisterClass(&wc)) {
		Msg(TEXT("RegisterClass Failed! Error=0x%x\r\n"), GetLastError());
		CoUninitialize();
		exit(1);
	}

	ghApp = CreateWindow(CLASSNAME, APPLICATIONNAME,
		WS_OVERLAPPEDWINDOW | WS_CAPTION | WS_CLIPCHILDREN | WS_VISIBLE,
		CW_USEDEFAULT, CW_USEDEFAULT,
		DEFAULT_PLAYER_WIDTH, DEFAULT_PLAYER_HEIGHT,
		0, 0, ghInst, 0);

	if (ghApp) {
		ghMenu = GetMenu(ghApp);
		EnableTickerMenu(FALSE);

		_tcsncpy(g_szAppText, BLEND_TEXT, NUMELMS(g_szAppText));
		g_dwTickerFlags = DEFAULT_MARK;

		if (g_szFileName[0] != 0)
			PostMessage(ghApp, WM_COMMAND, ID_FILE_INITCLIP, 0);

		while (GetMessage(&msg, NULL, 0, 0))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}
	else
	{
		Msg(TEXT("Failed to create the main window? Error=0x%x\r\n"), GetLastError());
	}
	CoUninitialize();
	return (int)msg.wParam;
}
