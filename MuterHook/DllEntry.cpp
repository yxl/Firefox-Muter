#include "DllEntry.h"
#include "ApiHooks.h"
#include "SDKTrace.h"

#include <time.h>

#ifdef __cplusplus
extern "C"
{
#endif				/* __cplusplus */


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

	CHAR g_szThisModulePath[MAX_PATH];
	DWORD g_dwThisModuleProcessId = 0;

	BOOL APIENTRY DllMain (HINSTANCE hInst     /* Library instance handle. */ ,
		DWORD reason        /* Reason this function is being called. */ ,
		LPVOID reserved     /* Not used. */ )
	{
		switch (reason)
		{
		case DLL_PROCESS_ATTACH:
			g_dwThisModuleProcessId = GetCurrentProcessId();
			GetModuleFileNameA(hInst, g_szThisModulePath, sizeof(g_szThisModulePath) / sizeof(CHAR));
			InstallMuterHooks();
			break;

		case DLL_PROCESS_DETACH:
			UnInstallMuterHooks();
			break;

		case DLL_THREAD_ATTACH:
			break;

		case DLL_THREAD_DETACH:
			break;
		}

		/* Returns TRUE on success, FALSE on failure */
		return TRUE;
	}

#ifdef __cplusplus
}
#endif				/* __cplusplus */