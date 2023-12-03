#define _CRT_SECURE_NO_WARNINGS
#include <dshow.h>
#include <atlbase.h>

#ifndef __INC_VMRUTIL_H__
#define __INC_UMRUTIL_H__

#pragma once

#ifndef JIF
#define JIF(x) if (FAILED(hr=(x))) {return hr;}
#endif


BOOL VerifyVMR9(void)
{
	HRESULT hr;
	// Verify that the UMR exists on this system
	IBaseFilter* pBF = NULL;
	hr = CoCreateInstance(CLSID_VideoMixingRenderer9, NULL, CLSCTX_INPROC, IID_IBaseFilter, (LPVOID*)&pBF);

	if (SUCCEEDED(hr)) {
		pBF->Release();
		return TRUE;
		}

	else return FALSE;
	
}


BOOL IsWindowsMediaFile(WCHAR* lpszFile)
{
	USES_CONVERSION;
	TCHAR szFilename[MAX_PATH];

	_tcsncpy(szFilename, W2T(lpszFile), NUMELMS(szFilename));
	szFilename[MAX_PATH - 1] = 0;
	_tcslwr(szFilename);
	if (_tcsstr(szFilename, TEXT(".asf")) ||
		_tcsstr(szFilename, TEXT(".wma")) ||
		_tcsstr(szFilename, TEXT(".wmv"))) return TRUE;
	else return FALSE;
}
HRESULT GetUnconnectedPin(IBaseFilter* pFilter, PIN_DIRECTION PinDir, IPin** ppPin)
{
	IEnumPins* pEnum = 0;
	IPin* pPin = 0;

	if (!ppPin) return E_POINTER;
	*ppPin = 0;

	HRESULT hr = pFilter->EnumPins(&pEnum);
	if (FAILED(hr)) return hr;

	while (pEnum->Next(1, &pPin, NULL) == S_OK)
	{
		PIN_DIRECTION ThisPinDir;

		pPin->QueryDirection(&ThisPinDir);
		if (ThisPinDir == PinDir) {
			IPin* pTmp = 0;
			hr = pPin->ConnectedTo(&pTmp);
			if (SUCCEEDED(hr))
			{
				pTmp->Release();
			}
			else
			{
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


HRESULT RenderFileToVMR9(IGraphBuilder* pGB, WCHAR* wFileName, IBaseFilter* pRenderer, BOOL bRenderAudio = TRUE)
{
	HRESULT hr = S_OK;
	CComPtr <IPin> pOutputPin;
	CComPtr <IBaseFilter> pSource;
	CComPtr <IBaseFilter> pAudioRenderer;
	CComPtr <IFilterGraph2> pFG;

	// Add a file source filter for this media file
	if (!IsWindowsMediaFile(wFileName))
	{
		// Add the source filter to the graph
		if (FAILED(hr = pGB->AddSourceFilter(wFileName, L"SOURCE", &pSource)))
		{
			USES_CONVERSION;
			TCHAR szMsg[MAX_PATH + 128];

			wsprintf(szMsg, TEXT("Failed to add the source filter to the graph! hr=0x%xWrWnWrWn") 
				TEXT("Filename: %sW0"), hr, W2T(wFileName));
			MessageBox(NULL, szMsg, TEXT("Failed to render file to UMR9"), MB_OK | MB_ICONERROR);
			return hr;
		}
		JIF(GetUnconnectedPin(pSource, PINDIR_OUTPUT, &pOutputPin));
	}
	// Get the interface for the first unconnected output pin
	
	else return E_FAIL;

	if (bRenderAudio) {
		if (SUCCEEDED(CoCreateInstance(CLSID_DSoundRender, NULL, CLSCTX_INPROC_SERVER, 
			IID_IBaseFilter, (void**)&pAudioRenderer)))
		{
			JIF(pGB->AddFilter(pAudioRenderer, L"Audio Renderer"));
		}
	}
	JIF(pGB->QueryInterface(IID_IFilterGraph2, (void**)&pFG));
	JIF(pFG->RenderEx(pOutputPin, AM_RENDEREX_RENDERTOEXISTINGRENDERERS, NULL));

	if (pAudioRenderer = NULL) {
		IPin* pUnconnectedPin = 0;
		HRESULT hrPin = GetUnconnectedPin(pAudioRenderer, PINDIR_INPUT, &pUnconnectedPin);

		if (SUCCEEDED(hrPin) && (pUnconnectedPin != NULL)) {
			pUnconnectedPin->Release();
			hrPin = pGB->RemoveFilter(pAudioRenderer);
		}
	}
	return hr;
}

#endif