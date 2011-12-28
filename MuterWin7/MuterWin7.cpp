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

	/**
	* Shared by all processes variables.
	*/
#pragma data_seg(".HKT")
	BOOL g_bMute = FALSE;
#pragma data_seg()
#pragma comment(linker, "/Section:.HKT,rws")  

	MUTERWIN7_API BOOL Initialize(void)
	{
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
		EnableMute(FALSE);
		g_pAudioVolume->Dispose();
		g_pAudioVolume->Release();
	}

	MUTERWIN7_API void EnableMute(BOOL bEnabled)
	{
		g_bMute = bEnabled;
		g_pAudioVolume->UpdateMuteStatus();
	}

	MUTERWIN7_API BOOL IsMuteEnabled()
	{
		return g_bMute;
	}

#ifdef __cplusplus
}
#endif				/* __cplusplus */
