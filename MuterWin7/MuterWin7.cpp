// MuterWin7.cpp : Defines the exported functions for the DLL application.
//

#include "stdafx.h"
#include "MuterWin7.h"
#include "AudioVolume.h"

#include <iostream>

#ifdef __cplusplus
extern "C"
{
#endif				/* __cplusplus */

	AudioVolume* g_pAudioVolume = NULL;

	MUTERWIN7_API BOOL Initialize(void)
	{
		HRESULT hr = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED | COINIT_MULTITHREADED);
		if (FAILED(hr))
		{
			return FALSE;
		}
		g_pAudioVolume = new (std::nothrow)AudioVolume();
		if (g_pAudioVolume == NULL) 
		{
			CoUninitialize();
			return FALSE;
		}
		g_pAudioVolume->Initialize();
		EnableMute(FALSE);
		return TRUE;
	}

	MUTERWIN7_API void Dispose(void)
	{
		if (g_pAudioVolume != NULL)
		{
			EnableMute(FALSE);
			g_pAudioVolume->Dispose();
			g_pAudioVolume->Release();
		}
		CoUninitialize();
	}

	MUTERWIN7_API void EnableMute(BOOL bEnabled)
	{
		if (g_pAudioVolume != NULL)
		{
			g_pAudioVolume->SetMuteStatus(bEnabled);
		}
	}

	MUTERWIN7_API BOOL IsMuteEnabled()
	{
		if (g_pAudioVolume != NULL)
		{
			return g_pAudioVolume->IsMuted();
		}
		return FALSE;
	}

	MUTERWIN7_API BOOL IsDefaultDeviceChanged()
	{
		if (g_pAudioVolume != NULL)
		{
			return g_pAudioVolume->IsDefaultDeviceChanged();
		}
		return FALSE;
	}
#ifdef __cplusplus
}
#endif				/* __cplusplus */
