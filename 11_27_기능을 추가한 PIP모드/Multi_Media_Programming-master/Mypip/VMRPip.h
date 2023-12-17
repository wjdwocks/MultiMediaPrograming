#include <d3d9.h>
#include <vmr9.h>

HRESULT InitPlayerWindow(void);
HRESULT InitVideoWindow(int nMultiplier, int nDivider);
HRESULT HandleGraphEvent(void);

BOOL GetClipFileName(LPSTR szName);
BOOL CheckVideoVisibility(void);

void MoveVideoWindow(void);
void CloseInterfaces(void);
void OpenClip(void);
void CloseFiles(void);
void OpenFiles(void);
void GetFilename(TCHAR* pszFull, TCHAR* pszFile);
void Msg(TCHAR* szFormat, ...);

HRESULT InitializeWindowlessVMR(IBaseFilter** ppVMR9);
HRESULT BlendVideo(LPSTR szFile1, LPSTR szf);
void SetNextQuadrant(int nStream);
void OnPaint(HWND hwnd);
void EnableMenus(BOOL bEnable);

LRESULT CALLBACK FilesDlgProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);

//#define FILE_FILTER_TEXT \TEXT("Video Files (*.asf; *.avi; *.qt; *.mov; *.mpg; *.mpeg; *.m1v; *.wmv)\0")
#define FILE_FILTER_TEXT TEXT("Video Files (*.asf; *.avi; *.qt; *.mov; *.mpg; *.mpeg; *.m1v; *.wmv)\0")

#define DEFAULT_MEDIA_PATH TEXT("\\\0")

#define DEFAULT_PLAYER_WIDTH	260
#define DEFAULT_PLAYER_HEIGHT	120

#define DEFAULT_VIDEO_WIDTH		320
#define DEFAULT_VIDEO_HEIGHT	240

#define MINIMUM_VIDEO_WIDTH	200
#define MINIMUM_VIDEO_HEIGHT	120

#define APPLICATIONNAME	TEXT("VMR9 Picture-In-Picture\0")
#define CLASSNAME		TEXT("VMR9RIP\0")

#define WM_GRAPHNOTIFY	WM_USER+1

extern HWND			ghApp;
extern HMENU		ghMenu;
extern HINSTANCE	ghInst;
extern DWORD		g_dwGraphRegister;

extern IGraphBuilder* pGB;
extern IMediaControl* pMC;
extern IVMRWindowlessControl9* pWC;
extern IMediaControl* pMC;
extern IMediaEventEx* pME;
extern IMediaSeeking* pMS;
extern IVMRMixerControl9* pMix;

#define SAFE_RELEASE(x) { if(x) x->Release(); x = NULL; }
//#define JIF(x) if (FAILED(hr=(x))) \{return hr;}
#define JIF(x) if (FAILED(hr=(x))) {return hr;}
//#define LIF(x) if (FAILED(hr=(x))) \{}
#define LIF(x) if (FAILED(hr=(x))) {}

//#define IDM_SWAP_ANIMATE 40006
//#define IDM_SWAP 40007