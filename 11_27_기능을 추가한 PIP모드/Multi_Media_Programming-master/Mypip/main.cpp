#define _CRT_SECURE_NO_WARNINGS
#include <dshow.h>
#include <tchar.h>
#include <commctrl.h>
#include <commdlg.h>
#include <stdio.h>
#include <atlbase.h>

#include "VMRPip.h"
#include "Blend.h"
#include "resource.h"

HWND		ghApp = 0;
HMENU		ghMenu = 0;
HINSTANCE	ghInst = 0;
DWORD		g_dwGraphRegister = 0;
RECT		g_rcDest = { 0 };
BOOL		g_bFilesSelected = FALSE;

IGraphBuilder* pGB = NULL;
IMediaControl* pMC = NULL;
IMediaEventEx* pME = NULL;
IMediaSeeking* pMS = NULL;

IVMRWindowlessControl9* pWC = NULL;
IVMRMixerControl9* pMix = NULL;

TCHAR g_szFile1[MAX_PATH] = { 0 }, g_szFile2[MAX_PATH] = { 0 };

const STRM_PARAM strParamInit[1] = {
	{0.0F, 0.0F, 1.0F, 1.0F, 1.0F, 0, 0, {0,0,0,0}}
};

STRM_PARAM strParam[2] = { 0 }, strDelta[2] = { 0 };

const QUADRANT quad[4] = {
	X_EDGE_BUFFER, Y_EDGE_BUFFER,
		SECONDARY_WIDTH, SECONDARY_HEIGHT,
	(1.0f - SECONDARY_WIDTH - X_EDGE_BUFFER), Y_EDGE_BUFFER,
		SECONDARY_WIDTH, SECONDARY_HEIGHT,
	(1.0f - SECONDARY_WIDTH - X_EDGE_BUFFER), (1.0f - SECONDARY_HEIGHT - Y_EDGE_BUFFER),
		SECONDARY_WIDTH, SECONDARY_HEIGHT,
	X_EDGE_BUFFER, (1.0f - SECONDARY_HEIGHT - Y_EDGE_BUFFER),
		SECONDARY_WIDTH, SECONDARY_HEIGHT,
};

int g_nCurrentQuadrant[2] = { 0 }, g_nSubpictureID = SECONDARY_STREAM;
int gnTimer = 0, g_nSwapSteps = 0, g_nSwapState = SWAP_NOTHING;

BOOL VerifyVMR9(void);
void EnableMenus(BOOL bEnable);
LRESULT CALLBACK WndMainProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
void OnPaint(HWND hwnd);
void CloseFiles();
void CloseInterfaces(void);
HRESULT InitPlayerWindow(void);
void OpenFiles(void);
LRESULT CALLBACK FilesDlgProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
BOOL GetClipFileName(LPSTR szName);
void InitStreamParams(void);
void PositionStream(int nStream, int nQuanrant);
HRESULT UpdatePinPos(int nStreamID);
HRESULT BlendVideo(LPSTR szFile1, LPSTR szFile2);
HRESULT ConfigureMultiFileVMR9(WCHAR* wFile1, WCHAR* wFile2);
void MoveVideoWindow();
HRESULT InitializeWindowlessVMR(IBaseFilter** ppVmr9);
HRESULT RenderFileToVMR9(IGraphBuilder* pGB, WCHAR* wFileName, IBaseFilter* pRenderer);
//HRESULT RenderFileToVMR9(IGraphBuilder *pGB, WCHAR *wFileName, IBaseFilter *pRenderer, BOOL bRenderAudio);
BOOL IsWindowsMediaFile(WCHAR* lpszFile);
HRESULT GetUnconnectedPin(IBaseFilter* pFilter, PIN_DIRECTION PinDir, IPin** ppPin);
BOOL CheckVideoVisibility();
HRESULT InitVideoWindow(int nMultiplier, int nDivider);
HRESULT UpdatePinAlpha(int nStreamID);
void SwapStreams();
void StartSwapAnimation(void);
void StartTimer(int nTimeout);
void StopTimer(void);
VOID CALLBACK TimerProc(HWND hwnd, UINT uMsg, UINT_PTR idEvent, DWORD dwTime);
void AdjustStreamRect(int nStream, STRM_PARAM* pStream);
void AdjustAlpha(int nStream, STRM_PARAM* pStream);
HRESULT UpdatePinZOrder(int nStreamID, DWORD dwZOrder);

BOOL VerifyVMR9(void)
{
	HRESULT hr;

	IBaseFilter* pBF = NULL;
	hr = CoCreateInstance(CLSID_VideoMixingRenderer9, NULL,
		CLSCTX_INPROC, IID_IBaseFilter, (LPVOID*)&pBF);
	if (SUCCEEDED(hr)) {
		pBF->Release();
		return TRUE;
	}
	else return FALSE;
}

void EnableMenus(BOOL bEnable) {
	int nState = (UINT)((bEnable) ? MF_ENABLED : MF_GRAYED) | MF_BYPOSITION;

	EnableMenuItem(ghMenu, 1, nState);
	EnableMenuItem(ghMenu, 2, nState);
	DrawMenuBar(ghApp);
}

LRESULT CALLBACK WndMainProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
	switch (message) {
	case WM_PAINT: OnPaint(hWnd);
		break;
	case WM_COMMAND:
		switch (wParam) {
		case ID_FILE_OPENVIDEOFILES: CloseFiles();
			OpenFiles();
			break;
		case ID_FILE_EXIT: CloseFiles();
			PostQuitMessage(0);
			break;
		case ID_FILE_CLOSEFILES: CloseFiles();
			break;
		case ID_EFFECT_SWAPSTREAMS: SwapStreams();
			break;
		case ID_EFFECT_ANIMATEDSTREAMSWAP: StartSwapAnimation();
			break;
		}
		break;
	case WM_CLOSE: SendMessage(ghApp, WM_COMMAND, ID_FILE_EXIT, 0);
		break;
	case WM_DESTROY: PostQuitMessage(0);
		break;
	default: return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return DefWindowProc(hWnd, message, wParam, lParam);
}

void OnPaint(HWND hwnd) {
	HRESULT hr;
	PAINTSTRUCT ps;
	HDC hdc;
	RECT rcClient;

	GetClientRect(hwnd, &rcClient);
	hdc = BeginPaint(hwnd, &ps);

	if (pWC) {
		hr = pWC->RepaintVideo(hwnd, hdc);
	}
	else {
		FillRect(hdc, &rcClient, (HBRUSH)(COLOR_BTNFACE + 1));
	}
	EndPaint(hwnd, &ps);
}

void CloseFiles() {
	HRESULT hr;

	if (pMC) hr = pMC->Stop();

	CloseInterfaces();
	EnableMenus(FALSE);

	RECT rect;
	GetClientRect(ghApp, &rect);
	InvalidateRect(ghApp, &rect, TRUE);

	InitPlayerWindow();
}

void CloseInterfaces(void) {
	if (pME)
	{
		pME->SetNotifyWindow((OAHWND)NULL, 0, 0);
	}

	SAFE_RELEASE(pME);
	SAFE_RELEASE(pMS);
	SAFE_RELEASE(pMC);
	SAFE_RELEASE(pWC);
	SAFE_RELEASE(pMix);
	SAFE_RELEASE(pGB);
}

HRESULT InitPlayerWindow(void) {
	SetWindowPos(ghApp, NULL, 0, 0, DEFAULT_PLAYER_WIDTH, DEFAULT_PLAYER_HEIGHT,
		SWP_NOMOVE | SWP_NOOWNERZORDER);
	return S_OK;
}

void OpenFiles(void) {
	HRESULT hr;

	DialogBox(ghInst, MAKEINTRESOURCE(IDD_DIALOG1), ghApp, (DLGPROC)FilesDlgProc);

	if (g_bFilesSelected) {
		g_bFilesSelected = FALSE;

		InitStreamParams();

		hr = BlendVideo(g_szFile1, g_szFile2);

		if (FAILED(hr)) {
			CloseFiles();
			return;
		}

		UpdatePinPos(0);
		UpdatePinPos(1);
		UpdatePinAlpha(0);
		UpdatePinAlpha(1);
	}
}

LRESULT CALLBACK FilesDlgProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
	static TCHAR szFile[MAX_PATH] = { 0 };

	switch (message) {
	case WM_INITDIALOG: SetDlgItemText(hWnd, IDC_EDIT2, g_szFile1);
		SetDlgItemText(hWnd, IDC_EDIT3, g_szFile2);
		return TRUE;
	case WM_COMMAND:
		switch (wParam) {
		case IDCANCEL: g_bFilesSelected = FALSE;
			EndDialog(hWnd, TRUE);
			break;
		case IDOK:
			GetDlgItemText(hWnd, IDC_EDIT2, g_szFile1, MAX_PATH);
			GetDlgItemText(hWnd, IDC_EDIT3, g_szFile2, MAX_PATH);
			g_bFilesSelected = TRUE;
			EndDialog(hWnd, TRUE);
			break;
		case IDC_BUTTON1:
			if (GetClipFileName(szFile))
				SetDlgItemText(hWnd, IDC_EDIT2, szFile);
			break;
		case IDC_BUTTON2:
			if (GetClipFileName(szFile))
				SetDlgItemText(hWnd, IDC_EDIT3, szFile);
			break;
		}
		break;
	}
	return FALSE;
}

BOOL GetClipFileName(LPSTR szName) {
	static OPENFILENAME ofn = { 0 };
	static BOOL bSetInitialDir = FALSE;

	*szName = 0;

	ofn.lStructSize = sizeof(OPENFILENAME);
	ofn.hwndOwner = ghApp;
	ofn.lpstrFilter = FILE_FILTER_TEXT;
	ofn.lpstrCustomFilter = NULL;
	ofn.nFilterIndex = 1;
	ofn.lpstrFile = szName;
	ofn.nMaxFile = MAX_PATH;
	ofn.lpstrTitle = TEXT("Open Video File...\0");
	ofn.lpstrFileTitle = NULL;
	ofn.lpstrDefExt = TEXT("*\0");
	ofn.Flags = OFN_FILEMUSTEXIST | OFN_READONLY;

	if (bSetInitialDir == FALSE) {
		ofn.lpstrInitialDir = DEFAULT_MEDIA_PATH;
		bSetInitialDir = TRUE;
	}
	else
	{
		ofn.lpstrInitialDir = NULL;
	}

	return GetOpenFileName((LPOPENFILENAME)&ofn);
}

void InitStreamParams(void) {
	CopyMemory(&strParam[PRIMARY_STREAM], strParamInit, sizeof(strParamInit));
	CopyMemory(&strParam[SECONDARY_STREAM], strParamInit, sizeof(strParamInit));

	strParam[1].fAlpha = TRANSPARENCY_VALUE;

	PositionStream(SECONDARY_STREAM, DEFAULT_QUADRANT);
}

void PositionStream(int nStream, int nQuadrant) {
	strParam[nStream].xPos = quad[nQuadrant].xPos;
	strParam[nStream].yPos = quad[nQuadrant].yPos;
	strParam[nStream].xSize = quad[nQuadrant].xSize;
	strParam[nStream].ySize = quad[nQuadrant].ySize;

	UpdatePinPos(nStream);

	g_nCurrentQuadrant[nStream] = nQuadrant;
}

HRESULT UpdatePinPos(int nStreamID) {
	HRESULT hr = S_OK;
	STRM_PARAM* p = &strParam[nStreamID];
	VMR9NormalizedRect r = { p->xPos, p->yPos, p->xPos + p->xSize, p->yPos + p->ySize };

	if (pMix) hr = pMix->SetOutputRect(nStreamID, &r);

	return hr;
}

HRESULT BlendVideo(LPSTR szFile1, LPSTR szFile2) {
	USES_CONVERSION;
	WCHAR wFile1[MAX_PATH], wFile2[MAX_PATH];
	HRESULT hr;

	if ((szFile1 == NULL) || (szFile2 == NULL)) return E_POINTER;

	UpdateWindow(ghApp);

	wcsncpy(wFile1, T2W(szFile1), NUMELMS(wFile1) - 1);
	wcsncpy(wFile2, T2W(szFile2), NUMELMS(wFile2) - 1);
	wFile1[MAX_PATH - 1] = wFile2[MAX_PATH - 1] = 0;

	JIF(CoCreateInstance(CLSID_FilterGraph, NULL, CLSCTX_INPROC_SERVER, IID_IGraphBuilder, (void**)&pGB));

	if (SUCCEEDED(hr)) {
		if (FAILED(ConfigureMultiFileVMR9(wFile1, wFile2))) return hr;

		JIF(pGB->QueryInterface(IID_IMediaControl, (void**)&pMC));
		JIF(pGB->QueryInterface(IID_IMediaEventEx, (void**)&pME));
		JIF(pGB->QueryInterface(IID_IMediaSeeking, (void**)&pMS));

		if (CheckVideoVisibility()) {
			JIF(InitVideoWindow(1, 1));
		}
		else {
			return E_FAIL;
		}

		JIF(pME->SetNotifyWindow((OAHWND)ghApp, WM_GRAPHNOTIFY, 0));

		ShowWindow(ghApp, SW_SHOWNORMAL);
		UpdateWindow(ghApp);
		SetForegroundWindow(ghApp);
		SetFocus(ghApp);

		MoveVideoWindow();
		JIF(pMC->Run());

		EnableMenus(TRUE);
	}
	return hr;
}

HRESULT ConfigureMultiFileVMR9(WCHAR* wFile1, WCHAR* wFile2) {
	HRESULT hr = S_OK;
	CComPtr <IBaseFilter> pVmr;

	JIF(InitializeWindowlessVMR(&pVmr));

	if (SUCCEEDED(hr = RenderFileToVMR9(pGB, wFile1, pVmr)))
		hr = RenderFileToVMR9(pGB, wFile2, pVmr);

	return hr;
}

void MoveVideoWindow() {
	HRESULT hr;

	if (pWC) {
		GetClientRect(ghApp, &g_rcDest);
		hr = pWC->SetVideoPosition(NULL, &g_rcDest);
	}
}

HRESULT InitializeWindowlessVMR(IBaseFilter** ppVmr9) {
	IBaseFilter* pVmr = NULL;

	HRESULT hr = CoCreateInstance(CLSID_VideoMixingRenderer9, NULL, CLSCTX_INPROC, IID_IBaseFilter, (void**)&pVmr);

	if (SUCCEEDED(hr)) {
		hr = pGB->AddFilter(pVmr, L"Video Mixing Renderer 9");
		if (SUCCEEDED(hr)) {
			CComPtr <IVMRFilterConfig9> pConfig;

			JIF(pVmr->QueryInterface(IID_IVMRFilterConfig9, (void**)&pConfig));
			JIF(pConfig->SetRenderingMode(VMR9Mode_Windowless));
			JIF(pConfig->SetNumberOfStreams(2));

			JIF(pVmr->QueryInterface(IID_IVMRWindowlessControl9, (void**)&pWC));
			JIF(pWC->SetVideoClippingWindow(ghApp));
			JIF(pWC->SetBorderColor(RGB(0, 0, 0)));

			JIF(pVmr->QueryInterface(IID_IVMRMixerControl9, (void**)&pMix));
		}
		*ppVmr9 = pVmr;
	}
	return hr;
}

//HRESULT RenderFileToVMR9(IGraphBuilder *pGB, WCHAR *wFileName, IBaseFilter *pRenderer, BOOL bRenderAudio = TRUE) {
HRESULT RenderFileToVMR9(IGraphBuilder* pGB, WCHAR* wFileName, IBaseFilter* pRenderer) {
	BOOL bRenderAudio = TRUE;
	HRESULT hr = S_OK;
	CComPtr <IPin> pOutputPin;
	CComPtr <IBaseFilter> pSource;
	CComPtr <IBaseFilter> pAudioRenderer;
	CComPtr <IFilterGraph2> pFG;

	if (!IsWindowsMediaFile(wFileName)) {
		if (FAILED(hr = pGB->AddSourceFilter(wFileName, L"SOURCE", &pSource))) {
			USES_CONVERSION;
			TCHAR szMsg[MAX_PATH + 128];

			wsprintf(szMsg, TEXT("Failed to add the source filter to the graph! hr=0x%x\r\n\r\n"), TEXT("Filename: %s\0"), hr, W2T(wFileName));
			MessageBox(NULL, szMsg, TEXT("Failed to render file to VMR9"), MB_OK | MB_ICONERROR);

			return hr;
		}

		JIF(GetUnconnectedPin(pSource, PINDIR_OUTPUT, &pOutputPin));
	}
	else {
		return E_FAIL;
	}

	if (bRenderAudio) {
		if (SUCCEEDED(CoCreateInstance(CLSID_DSoundRender, NULL, CLSCTX_INPROC_SERVER, IID_IBaseFilter, (void**)&pAudioRenderer))) {
			JIF(pGB->AddFilter(pAudioRenderer, L"Audio Renderer"));
		}
	}

	JIF(pGB->QueryInterface(IID_IFilterGraph2, (void**)&pFG));

	JIF(pFG->RenderEx(pOutputPin, AM_RENDEREX_RENDERTOEXISTINGRENDERERS, NULL));

	if (pAudioRenderer != NULL) {
		IPin* pUnconnectedPin = 0;

		HRESULT hrPin = GetUnconnectedPin(pAudioRenderer, PINDIR_INPUT, &pUnconnectedPin);

		if (SUCCEEDED(hrPin) && (pUnconnectedPin != NULL)) {
			pUnconnectedPin->Release();

			hrPin = pGB->RemoveFilter(pAudioRenderer);
		}
	}
	return hr;
}

BOOL IsWindowsMediaFile(WCHAR* lpszFile) {
	USES_CONVERSION;
	TCHAR szFilename[MAX_PATH];

	_tcsncpy(szFilename, W2T(lpszFile), NUMELMS(szFilename));
	szFilename[MAX_PATH - 1] = 0;
	_tcslwr(szFilename);

	if (_tcsstr(szFilename, TEXT(".asf")) ||
		_tcsstr(szFilename, TEXT(".wma")) ||
		_tcsstr(szFilename, TEXT(".wmv")))
		return TRUE;
	else return FALSE;
}

HRESULT GetUnconnectedPin(IBaseFilter* pFilter, PIN_DIRECTION PinDir, IPin** ppPin) {
	IEnumPins* pEnum = 0;
	IPin* pPin = 0;

	if (!ppPin) return E_POINTER;
	*ppPin = 0;

	HRESULT hr = pFilter->EnumPins(&pEnum);
	if (FAILED(hr)) return hr;

	while (pEnum->Next(1, &pPin, NULL) == S_OK) {
		PIN_DIRECTION ThisPinDir;

		pPin->QueryDirection(&ThisPinDir);
		if (ThisPinDir == PinDir) {
			IPin* pTmp = 0;

			hr = pPin->ConnectedTo(&pTmp);
			if (SUCCEEDED(hr))
				pTmp->Release();
			else {
				pEnum->Release();
				*ppPin = pPin;
				return S_OK;
			}
		}
		pPin->Release();
	}
	pEnum->Release();
	return E_FAIL;
}

BOOL CheckVideoVisibility() {
	HRESULT hr;
	LONG lWidth = 0, lHeight = 0;

	if (!pWC) {
		return FALSE;
	}

	hr = pWC->GetNativeVideoSize(&lWidth, &lHeight, 0, 0);
	if (FAILED(hr)) {
		return FALSE;
	}

	if ((lWidth == 0) && (lHeight == 0)) return FALSE;

	return TRUE;
}

HRESULT InitVideoWindow(int nMultiplier, int nDivider) {
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

	SetWindowPos(ghApp, NULL, 0, 0, lWidth + 2 * nBorderWidth, lHeight + nTitleHeight + 2 * nBorderHeight, SWP_NOMOVE | SWP_NOOWNERZORDER);

	GetClientRect(ghApp, &g_rcDest);
	hr = pWC->SetVideoPosition(NULL, &g_rcDest);

	return hr;
}

HRESULT UpdatePinAlpha(int nStreamID) {
	HRESULT hr = S_OK;

	STRM_PARAM* p = &strParam[nStreamID];

	if (pMix) hr = pMix->SetAlpha(nStreamID, p->fAlpha);

	return hr;
}

void SwapStreams() {
	STRM_PARAM strSwap = { 0 };

	strSwap = strParam[PRIMARY_STREAM];
	strParam[PRIMARY_STREAM] = strParam[SECONDARY_STREAM];
	strParam[SECONDARY_STREAM] = strSwap;

	UpdatePinPos(PRIMARY_STREAM);
	UpdatePinPos(SECONDARY_STREAM);

	UpdatePinAlpha(PRIMARY_STREAM);
	UpdatePinAlpha(SECONDARY_STREAM);

	g_nSubpictureID ^= 1;

	UpdatePinZOrder(!g_nSubpictureID, ZORDER_FAR);
	UpdatePinZOrder(g_nSubpictureID, ZORDER_CLOSE);
}

void StartSwapAnimation(void) {
	strDelta[0].xPos = (strParam[1].xPos - strParam[0].xPos) / NUM_ANIMATION_STEPS;
	strDelta[0].yPos = (strParam[1].yPos - strParam[0].yPos) / NUM_ANIMATION_STEPS;
	strDelta[0].xSize = (strParam[1].xSize - strParam[0].xSize) / NUM_ANIMATION_STEPS;
	strDelta[0].ySize = (strParam[1].ySize - strParam[0].ySize) / NUM_ANIMATION_STEPS;

	strDelta[1].xPos = (strParam[0].xPos - strParam[1].xPos) / NUM_ANIMATION_STEPS;
	strDelta[1].yPos = (strParam[0].yPos - strParam[1].yPos) / NUM_ANIMATION_STEPS;
	strDelta[1].xSize = (strParam[0].xSize - strParam[1].xSize) / NUM_ANIMATION_STEPS;
	strDelta[1].ySize = (strParam[0].ySize - strParam[1].ySize) / NUM_ANIMATION_STEPS;

	strDelta[0].fAlpha = (strParam[1].fAlpha - strParam[0].fAlpha) / NUM_ANIMATION_STEPS;
	strDelta[1].fAlpha = (strParam[0].fAlpha - strParam[1].fAlpha) / NUM_ANIMATION_STEPS;

	g_nSwapSteps = 0;

	g_nSwapState = SWAP_POSITION;
	StartTimer(POSITION_TIMEOUT);
}

void StartTimer(int nTimeout) {
	gnTimer = (int)SetTimer(NULL, UPDATE_TIMER, nTimeout, TimerProc);
}

void StopTimer(void) {
	if (gnTimer) {
		KillTimer(NULL, gnTimer);
		gnTimer = 0;
	}
}

VOID CALLBACK TimerProc(HWND hwnd, UINT uMsg, UINT_PTR idEvent, DWORD dwTime) {
	g_nSwapSteps++;

	if (g_nSwapState == SWAP_POSITION) {
		AdjustStreamRect(PRIMARY_STREAM, &strDelta[0]);
		AdjustStreamRect(SECONDARY_STREAM, &strDelta[1]);

		if (g_nSwapSteps == (int)NUM_ANIMATION_STEPS) {
			StopTimer();
			g_nSubpictureID ^= 1;

			UpdatePinZOrder(!g_nSubpictureID, ZORDER_FAR);
			UpdatePinZOrder(g_nSubpictureID, ZORDER_CLOSE);

			g_nSwapSteps = 0;

			g_nSwapState = SWAP_ALPHA;
			StartTimer(ALPHA_TIMEOUT);
		}
	}
	else if (g_nSwapState == SWAP_ALPHA) {
		AdjustAlpha(PRIMARY_STREAM, &strDelta[0]);
		AdjustAlpha(SECONDARY_STREAM, &strDelta[1]);

		if (g_nSwapSteps == (int)NUM_ANIMATION_STEPS) {
			StopTimer();

			g_nSwapState = SWAP_NOTHING;
			g_nSwapSteps = 0;
		}
	}
}

void AdjustStreamRect(int nStream, STRM_PARAM* pStream) {
	if (!pStream) return;

	strParam[nStream].xPos += pStream->xPos;
	strParam[nStream].yPos += pStream->yPos;
	strParam[nStream].xSize += pStream->xSize;
	strParam[nStream].ySize += pStream->ySize;

	UpdatePinPos(nStream);
}

void AdjustAlpha(int nStream, STRM_PARAM* pStream) {
	if (!pStream) return;

	strParam[nStream].fAlpha += pStream->fAlpha;

	UpdatePinAlpha(nStream);
}

HRESULT UpdatePinZOrder(int nStreamID, DWORD dwZOrder) {
	HRESULT hr = S_OK;

	if (pMix)
		hr = pMix->SetZOrder(nStreamID, dwZOrder);

	return hr;
}

int PASCAL WinMain(HINSTANCE hInstC, HINSTANCE hInstP, LPSTR lpCmdLine, int nCmdShow) {
	MSG msg = { 0 };
	WNDCLASS wc;

	if (FAILED(CoInitializeEx(NULL, COINIT_APARTMENTTHREADED))) return FALSE;

	if (!VerifyVMR9()) return FALSE;

	ZeroMemory(&wc, sizeof(wc));
	ghInst = wc.hInstance = hInstC;
	wc.lpfnWndProc = WndMainProc;
	wc.lpszClassName = CLASSNAME;
	wc.lpszMenuName = MAKEINTRESOURCE(IDR_MENU1);
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);

	if (!RegisterClass(&wc)) {
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
		EnableMenus(FALSE);

		while (GetMessage(&msg, NULL, 0, 0)) {
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}

	CoUninitialize();

	return (int)msg.wParam;
}