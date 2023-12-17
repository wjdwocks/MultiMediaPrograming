#define _CRT_SECURE_NO_WARNINGS
#include <dshow.h>
#include <stdio.h>

char g_FileName[256];
char g_PathFileName[512];

BOOL GetMediaFileName(void)
{
	OPENFILENAME ofn;

	ofn.lStructSize = sizeof(OPENFILENAME);
	ofn.hwndOwner = NULL;
	ofn.hInstance = NULL;
	ofn.lpstrFilter = NULL;
	ofn.lpstrCustomFilter = NULL;
	ofn.nMaxCustFilter = NULL;
	ofn.nFilterIndex = 0;
	ofn.lpstrFile = (char*)calloc(1, 512);
	ofn.nMaxFile = 511;
	ofn.lpstrFileTitle = (char*)calloc(1, 256);
	ofn.nMaxFileTitle = 255;
	ofn.lpstrInitialDir = NULL;
	ofn.lpstrTitle = "Select file to render...";
	ofn.Flags = OFN_FILEMUSTEXIST | OFN_HIDEREADONLY | OFN_PATHMUSTEXIST | OFN_NOCHANGEDIR;
	ofn.nFileOffset = 0;
	ofn.nFileExtension = 0;
	ofn.lpstrDefExt = NULL;
	ofn.lCustData = NULL;
	if (!GetOpenFileName(&ofn)) {
		free(ofn.lpstrFile);
		free(ofn.lpstrFileTitle);
		return false;
	}
	else {
		strcpy(g_PathFileName, ofn.lpstrFile);
		strcpy(g_FileName, ofn.lpstrFileTitle);
		free(ofn.lpstrFile);
		free(ofn.lpstrFileTitle);
	}
	return true;
}

HRESULT saveGraphFile(IGraphBuilder* pGraph, WCHAR* wszPath)
{
	const WCHAR wszStramName[] = L"ActiveMovieGraph";
	HRESULT hr;
	IStorage* pStroage = NULL;

	hr = StgCreateDocfile(
		wszPath,
		STGM_CREATE | STGM_TRANSACTED | STGM_READWRITE | STGM_SHARE_EXCLUSIVE, 0, &pStroage);
	if (FAILED(hr)) {
		return hr;
	}
}

IPin* GetPin(IBaseFilter* pFilter, PIN_DIRECTION PinDir) {
	BOOL bFound = FALSE;
	IEnumPins* pEnum;
	IPin* pPin;

	HRESULT hr = pFilter->EnumPins(&pEnum);
	if (FAILED(hr)) {
		return NULL;
	}
	while (pEnum->Next(1,&pPin,0)==S_OK)
	{
		PIN_DIRECTION pinDirThis;
		pPin->QueryDirection(&pinDirThis);
		if (bFound = (PinDir == pinDirThis))
			break;
		pPin->Release();
	}
	pEnum->Release();
	return (bFound ? pPin : NULL);
}


int main(int argc, char* argv[])
{
	IGraphBuilder* pGraph = NULL;
	IMediaControl* pControl = NULL;
	IMediaEvent* pEvent = NULL;
	IBaseFilter* pInputFileFilter = NULL;
	//비디오 디코딩, 렌더링을 위한 필터
	//IBaseFilter* pVideoDecoder = NULL;
	//IBaseFilter* pVideoRenderer = NULL;
	IBaseFilter* pDSoundRenderer = NULL;
	IPin* pFileOut = NULL, * pWAVIn = NULL;

	if (!GetMediaFileName()) {
		return 0;
	}


	HRESULT hr = CoInitialize(NULL);
	if (FAILED(hr))
	{
		printf("ERROR - Could not initialize COM library");
		return hr;
	}
	hr = CoCreateInstance(CLSID_FilterGraph, NULL, CLSCTX_INPROC_SERVER, IID_IGraphBuilder, (void**)& pGraph);
	if (FAILED(hr))
	{
		printf("ERROR - COuld not create the Filter Graph Manager.\n");
		CoUninitialize();
		return hr;
	}
	//Now get the media control object
	hr = pGraph->QueryInterface(IID_IMediaControl, (void**)&pControl);
	if (FAILED(hr))
	{
		pGraph->Release();
		CoUninitialize();
		return hr;
	}

	//And the media event object
	hr = pGraph->QueryInterface(IID_IMediaEvent, (void**)&pEvent);
	if (FAILED(hr))
	{
		pControl->Release();
		pGraph->Release();
		CoUninitialize();
		return hr;
	}

#ifndef  UNICODE
	WCHAR wFileName[MAX_PATH];
	MultiByteToWideChar(CP_ACP, 0, g_PathFileName, -1, wFileName, MAX_PATH);
	hr = pGraph->AddSourceFilter(wFileName,wFileName,&pInputFileFilter);
#else
	hr = pGraph->AddSourceFilter(wFileName, wFileName, &pInputFileFilter);
#endif // ! UNICODE

	//audio
	if (SUCCEEDED(hr))
	{
		hr = CoCreateInstance(CLSID_DSoundRender, NULL, CLSCTX_INPROC_SERVER, IID_IBaseFilter, (void**)&pDSoundRenderer);
		
		if (SUCCEEDED(hr)) {
			hr = pGraph->AddFilter(pDSoundRenderer, L"Audio Renderer");

			if (SUCCEEDED(hr)) {
				pFileOut = GetPin(pInputFileFilter, PINDIR_OUTPUT);
				if (pFileOut != NULL) {
					pWAVIn = GetPin(pDSoundRenderer, PINDIR_INPUT);
					if (pWAVIn != NULL) {
						hr = pGraph->Connect(pFileOut, pWAVIn);
					}
				}
			}
		}
	}




	if (SUCCEEDED(hr)) {
		printf("Beginning to play media file...\n");
		hr = pControl->Run();
		if (SUCCEEDED(hr)) {
			long evCode;
			pEvent->WaitForCompletion(INFINITE, &evCode);
		}
		hr = pControl->Stop();
	}

	saveGraphFile(pGraph, L"C:\\Users\\user\\Desktop\\Project2\\asdkj.grf");
	
	if (pFileOut) {
		pFileOut->Release();
	}
	if (pWAVIn) {
		pWAVIn->Release();
	}
	if (pInputFileFilter) {
		pInputFileFilter->Release();
	}
	if (pDSoundRenderer) {
		pDSoundRenderer->Release();
	}


	pControl->Release();
	pEvent->Release();
	pGraph->Release();
	CoUninitialize();

	return 0;
}

