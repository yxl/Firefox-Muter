#include "DllEntry.h"
#include "ApiHooks.h"
#include "SDKTrace.h"
#include "AudioVolume.h"

#include <time.h>
#include <iostream>

#ifdef __cplusplus
extern "C"
{
#endif				/* __cplusplus */

	CHAR g_szThisModulePath[MAX_PATH];
	DWORD g_dwThisModuleProcessId = 0;

	AudioVolume* g_pAudioVolume = NULL;
	DWORD g_dwOSVersion = 0;

	DWORD GetMajorOSVersion();

	/**
	* Shared by all processes variables.
	*/
#pragma data_seg(".HKT")
	BOOL g_bMute = FALSE;
	LONG g_lLastSoundPlayingTimeInSeconds = -1;
#pragma data_seg()
#pragma comment(linker, "/Section:.HKT,rws")  



	/*DLLIMPORT*/ void EnableMute(BOOL bEnabled)
	{
		if (g_dwOSVersion >= 6)
		{
			if (g_pAudioVolume == NULL)
			{
				g_pAudioVolume = new (std::nothrow)AudioVolume();
				if (g_pAudioVolume)
				{
					g_pAudioVolume->Initialize();
				}	
			}
			g_pAudioVolume->EnableMute(bEnabled);
		}
		g_bMute = bEnabled;
	}

	/*DLLIMPORT*/ BOOL IsMuteEnabled()
	{
		return g_bMute;
	}


	/*DLLIMPORT*/ LONG GetLastSoundPlayingTimeInSeconds()
	{
		return g_lLastSoundPlayingTimeInSeconds;
	}

	BOOL ShouldMute()
	{
		g_lLastSoundPlayingTimeInSeconds = (LONG)time(NULL);
		return g_bMute;
	}

	BOOL APIENTRY DllMain (HINSTANCE hInst     /* Library instance handle. */ ,
		DWORD reason        /* Reason this function is being called. */ ,
		LPVOID reserved     /* Not used. */ )
	{

		switch (reason)
		{
		case DLL_PROCESS_ATTACH:
			g_dwOSVersion = GetMajorOSVersion();

			if (g_dwOSVersion < 6)
			{
				g_dwThisModuleProcessId = GetCurrentProcessId();
				GetModuleFileNameA(hInst, g_szThisModulePath, sizeof(g_szThisModulePath) / sizeof(CHAR));
				InstallMuterHooks();
			}
			else
			{
				HRESULT hr = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);
				if (FAILED(hr))
				{
					return FALSE;
				}			

			}
			break;

		case DLL_PROCESS_DETACH:
			if (g_dwOSVersion < 6)
			{
				UnInstallMuterHooks();
			}
			else
			{
				if (g_pAudioVolume)
				{
					g_pAudioVolume->Dispose();
					g_pAudioVolume->Release();
				}
				CoUninitialize();
			}
			break;

		case DLL_THREAD_ATTACH:
			break;

		case DLL_THREAD_DETACH:
			break;
		}

		/* Returns TRUE on success, FALSE on failure */
		return TRUE;
	}

	/**
	* Get the major system version.
	* 5 -- Windows 2000/XP/2003
	* 6 -- Windows Vista/2008/7
	*/
	DWORD GetMajorOSVersion() 
	{
		OSVERSIONINFOEX info;
		info.dwOSVersionInfoSize = sizeof(info);
		GetVersionEx(reinterpret_cast<OSVERSIONINFO*>(&info));
		return info.dwMajorVersion;
	}

#ifdef __cplusplus
}
#endif				/* __cplusplus */