#pragma once
#pragma once

int PASCAL WinMain(HINSTANCE hInstance, HINSTANCE hInstp, LPSTR lpCmdLine, int nCmdShow);
LRESULT CALLBACK WndMainProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);

HRESULT GetInterfaces(void);
HRESULT CaptureVideo();
HRESULT FindCaptureDevice(IBaseFilter** ppSrcFilter);
HRESULT SetupVideoWindow(void);
HRESULT ChangePreviewState(int nShow);

void Msg(TCHAR* szFormat, ...);
void CloseInterfaces(void);
void ResizeVideoWindow(void);

enum PLAYSTATE { Stopped, Paused, Running, Init };


#define SAFE_RELEASE(x){if (x) x->Release(); x = NULL;}
#define JIF(x) if (FAILED(hr=(x)))\
    {Msg(TEXT("FAILED(hr=0x%x) in ") TEXT(#x) TEXT("\n\0"), hr); return hr;}


#define DEFAULT_VIDEO_WIDTH 320
#define DEFAULT_VIDEO_HEIGHT 320

#define APPLICAITONNAME TEXT("Video Capture Previewer (PlayCap)\0")
#define CLASSNAME        TEXT("VidCapPreviewer\0")

#define WM_GRAPHNOTIFY WM_APP+1

#define IDI_VIDPREVIEW 100