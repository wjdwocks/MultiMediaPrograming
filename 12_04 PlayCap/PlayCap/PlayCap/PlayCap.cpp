#define _CRT_SECURE_NO_WARNINGS
#include <atlbase.h>
#include <windows.h>
#include <dshow.h>
#include <stdio.h>

#include "PlayCap.h"

HWND ghApp = 0;
IVideoWindow* g_pVW = NULL;
IMediaControl* g_pMC = NULL;
IMediaEventEx* g_pME = NULL;
IGraphBuilder* g_pGraph = NULL;
ICaptureGraphBuilder2* g_pCapture = NULL;
PLAYSTATE g_psCurrent = Stopped;

HRESULT CaptureVideo()
{
	HRESULT hr;
	IBaseFilter* pSrcFilter = NULL;

	hr = GetInterfaces();
	if (FAILED(hr))
	{
		Msg(TEXT("Failed to get video interfaces! hr=0x%x"), hr);
		return hr;
	}

	hr = g_pCapture->SetFiltergraph(g_pGraph);
	if (FAILED(hr))
	{
		Msg(TEXT("Failed to set capture filter graph! hr=0x%x"), hr);
		return hr;
	}

	hr = FindCaptureDevice(&pSrcFilter);
	if (FAILED(hr))
	{
		return hr;
	}

	hr = g_pGraph->AddFilter(pSrcFilter, L"Video Capture");
	if (FAILED(hr))
	{
		Msg(TEXT("Couldn't add the capture filter to the graph! hr = 0x%x\r\n\r\n")
			TEXT("If you have a working video capture device, please make sure\r\n")
			TEXT("That it is connected and is not being used by another application.\r\n\r\n")
			TEXT("The sample will now close."), hr);
		pSrcFilter->Release();
		return hr;
	}

	pSrcFilter->Release();


	hr = SetupVideoWindow();
	if (FAILED(hr))
	{
		Msg(TEXT("Couldn't initialize video window! hr=0x%x"), hr);
		return hr;
	}
	g_psCurrent = Running;

	return S_OK;
}

HRESULT FindCaptureDevice(IBaseFilter** ppSrcFilter)
{
	HRESULT hr;
	IBaseFilter* pSrc = NULL;
	CComPtr <IMoniker> pMoniker = NULL;
	ULONG cFetched;

	if (!ppSrcFilter)
		return E_POINTER;

	CComPtr<ICreateDevEnum> pDevEnum = NULL;

	hr = CoCreateInstance(CLSID_SystemDeviceEnum, NULL, CLSCTX_INPROC, IID_ICreateDevEnum, (void**)&pDevEnum);

	if (FAILED(hr))
	{
		Msg(TEXT("Couldn't create system enumerator! hr=0x%x"), hr);
		return hr;
	}

	CComPtr<IEnumMoniker> pClassEnum = NULL;

	hr = pDevEnum->CreateClassEnumerator(CLSID_VideoInputDeviceCategory, &pClassEnum, 0);
	if (FAILED(hr))
	{
		Msg(TEXT("Couldn't create class enumerator! hr= 0x%x"), hr);
		return hr;
	}

	if (pClassEnum == NULL)
	{
		MessageBox(ghApp, TEXT("No video capture device was detected.\r\n\r\n")
			TEXT("This sample requires a video capture device, such as a USB WebCam, \r\n")
			TEXT("to be installed and working properly. The sample will now close."),
			TEXT("No Video Capture Hardware"), MB_OK | MB_ICONINFORMATION);
		return E_FAIL;
	}

	if (S_OK == (pClassEnum->Next(1, &pMoniker, &cFetched))) {
		hr = pMoniker->BindToObject(0, 0, IID_IBaseFilter, (void**)&pSrc);
		if (FAILED(hr))
		{
			Msg(TEXT("Couldn't bind moniker to filter object! hr=0x%x"), hr);
			return hr;
		}
	}
	else
	{
		Msg(TEXT("Unable to access video capture device!"));
		return E_FAIL;
	}

	*ppSrcFilter = pSrc;

	return hr;
}


HRESULT GetInterfaces(void)
{
	HRESULT hr;
	hr = CoCreateInstance(CLSID_FilterGraph, NULL, CLSCTX_INPROC, IID_IGraphBuilder, (void**)&g_pGraph);
	if (FAILED(hr)) return hr;

	hr = CoCreateInstance(CLSID_CaptureGraphBuilder2, NULL, CLSCTX_INPROC, IID_ICaptureGraphBuilder2, (void**)&g_pCapture);
	if (FAILED(hr)) return hr;

	hr = g_pGraph->QueryInterface(IID_IMediaControl, (LPVOID*)&g_pMC);
	if (FAILED(hr)) return hr;

	hr = g_pGraph->QueryInterface(IID_IVideoWindow, (LPVOID*)&g_pVW);
	if (FAILED(hr)) return hr;

	hr = g_pGraph->QueryInterface(IID_IMediaEvent, (LPVOID*)&g_pME);
	if (FAILED(hr)) return hr;

	return hr;
}

void CloseInterfaces(void)
{
	if (g_pMC) g_pMC->StopWhenReady();

	g_psCurrent = Stopped;

	if (g_pME) g_pME->SetNotifyWindow(NULL, WM_GRAPHNOTIFY, 0);

	if (g_pVW) {
		g_pVW->put_Visible(OAFALSE);
		g_pVW->put_Owner(NULL);
	}
	SAFE_RELEASE(g_pMC);
	SAFE_RELEASE(g_pME);
	SAFE_RELEASE(g_pVW);
	SAFE_RELEASE(g_pGraph);
	SAFE_RELEASE(g_pCapture);
}

HRESULT SetupVideoWindow(void)
{
	HRESULT hr;


	hr = g_pVW->put_Owner((OAHWND)ghApp);
	if (FAILED(hr)) return hr;

	hr = g_pVW->put_WindowStyle(WS_CHILD | WS_CLIPCHILDREN);
	if (FAILED(hr)) return hr;

	ResizeVideoWindow();

	hr = g_pVW->put_Visible(OATRUE);
	if (FAILED(hr)) return hr;

	return hr;
}


void ResizeVideoWindow(void)
{
	if (g_pVW) {
		RECT rc;

		GetClientRect(ghApp, &rc);
		g_pVW->SetWindowPosition(0, 0, rc.right, rc.bottom);
	}
}

HRESULT ChangePreviewState(int nShow)
{
	HRESULT hr = S_OK;

	if (!g_pMC) return S_OK;

	if (nShow) {
		if (g_psCurrent != Running) {
			hr = g_pMC->Run();
			g_psCurrent = Running;
		}
	}
	else {
		hr = g_pMC->StopWhenReady();
		g_psCurrent = Stopped;
	}
	return hr;
}

void Msg(TCHAR* szFormat, ...)
{
	TCHAR szBuffer[1024];
	const size_t NUMCHARS = sizeof(szBuffer) / sizeof(szBuffer[0]);
	const int LASTCHAR = NUMCHARS - 1;

	va_list pArgs;
	va_start(pArgs, szFormat);

	_vsnprintf(szBuffer, NUMCHARS - 1, szFormat, pArgs);

	szBuffer[LASTCHAR] = TEXT('\0');
	MessageBox(NULL, szBuffer, TEXT("PlayCap Message"), MB_OK | MB_ICONERROR);
}

LRESULT CALLBACK WndMainProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case WM_SIZE:
		ResizeVideoWindow();
		break;
	case WM_WINDOWPOSCHANGED:
		ChangePreviewState(!(IsIconic(hwnd)));
		break;
	case WM_CLOSE:
		ShowWindow(ghApp, SW_HIDE);
		CloseInterfaces();
		break;
	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;
	}
	if (g_pVW)
		g_pVW->NotifyOwnerMessage((LONG_PTR)hwnd, message, wParam, lParam);
	return DefWindowProc(hwnd, message, wParam, lParam);
}

//int PASCAL WinMain(HINSTANCE hInstance, HINSTANCE hInstP, LPSTR lpCmdLine, int nCmdShow)
//{
//	MSG msg = { 0 };
//	WNDCLASS wc;
//
//	if (FAILED(CoInitialize(NULL)))
//	{
//		Msg(TEXT("CoInitialize Failed!\r\n"));
//		exit(1);
//	}
//	ZeroMemory(&wc, sizeof wc);
//	wc.lpfnWndProc = WndMainProc;
//	wc.hInstance = hInstance;
//	wc.lpszClassName = CLASSNAME;
//	wc.lpszMenuName = NULL;
//	wc.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
//	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
//	wc.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_VIDPREVIEW));
//	if (!RegisterClass(&wc)) {
//		Msg(TEXT("RegisterClass Faild! Error = 0x%x\r\n"), GetLastError());
//		CoUninitialize();
//		exit(1);
//	}
//
//	ghApp = CreateWindow(CLASSNAME, APPLICAITONNAME,
//		WS_OVERLAPPEDWINDOW | WS_CAPTION | WS_CLIPCHILDREN,
//		CW_USEDEFAULT, CW_USEDEFAULT,
//		DEFAULT_VIDEO_WIDTH, DEFAULT_VIDEO_HEIGHT,
//		0, 0, hInstance, 0);
//
//	if (ghApp) {
//		HRESULT hr;
//
//		hr = CaptureVideo();
//		if (FAILED(hr)) {
//			CloseInterfaces();
//			DestroyWindow(ghApp);
//		}
//		else {
//			ShowWindow(ghApp, nCmdShow);
//		}
//		while (GetMessage(&msg, NULL, 0, 0)) {
//			TranslateMessage(&msg);
//			DispatchMessage(&msg);
//		}
//	}
//	CoUninitialize(); 
//	return (int)msg.wParam;
//}

int main()
{
	MSG msg = { 0 };
	WNDCLASS wc;

	if (FAILED(CoInitialize(NULL)))
	{
		MessageBox(NULL, TEXT("CoInitialize Failed!\r\n"), TEXT("Error"), MB_OK);
		return 1;
	}

	ZeroMemory(&wc, sizeof wc);
	wc.lpfnWndProc = WndMainProc;
	wc.hInstance = GetModuleHandle(NULL);
	wc.lpszClassName = CLASSNAME;
	wc.lpszMenuName = NULL;
	wc.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	wc.hIcon = LoadIcon(GetModuleHandle(NULL), MAKEINTRESOURCE(IDI_VIDPREVIEW));

	if (!RegisterClass(&wc)) {
		MessageBox(NULL, TEXT("RegisterClass Failed! Error = 0x%x\r\n"), TEXT("Error"), MB_OK | MB_ICONERROR);
		CoUninitialize();
		return 1;
	}

	ghApp = CreateWindow(CLASSNAME, APPLICAITONNAME,
		WS_OVERLAPPEDWINDOW | WS_CAPTION | WS_CLIPCHILDREN,
		CW_USEDEFAULT, CW_USEDEFAULT,
		DEFAULT_VIDEO_WIDTH, DEFAULT_VIDEO_HEIGHT,
		0, 0, GetModuleHandle(NULL), 0);

	if (ghApp) {
		HRESULT hr;

		hr = CaptureVideo();
		if (FAILED(hr)) {
			CloseInterfaces();
			DestroyWindow(ghApp);
		}
		else {
			ShowWindow(ghApp, SW_SHOWNORMAL);
		}

		while (GetMessage(&msg, NULL, 0, 0)) {
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}

	CoUninitialize();
	return (int)msg.wParam;
}