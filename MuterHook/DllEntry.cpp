#include "ThTypes.h"
#include "DllEntry.h"
#include "ApiHooks.h"

#ifdef __cplusplus
extern "C"
{
#endif				/* __cplusplus */

	HINSTANCE g_hInstance = NULL;

	/*DLLIMPORT*/ void EnableMute(BOOL bEnabled)
	{
		if (GlobalData)
		{
			GlobalData->bMute = bEnabled;
		}
	}

	/*DLLIMPORT*/ BOOL IsMuteEnabled()
	{
		BOOL bEnabled = FALSE;
		if (GlobalData)
		{
			bEnabled = GlobalData->bMute;
		}
		return bEnabled;
	}


	/*DLLIMPORT*/ LONG GetLastSoundPlayingTimeInSeconds()
	{
		LONG time = -1;
		if (GlobalData)
		{
			time = GlobalData->lLastSoundPlayingTimeInSeconds; 
		}
		return time;
	}


	BOOL APIENTRY DllMain (HINSTANCE hInst     /* Library instance handle. */ ,
		DWORD reason        /* Reason this function is being called. */ ,
		LPVOID reserved     /* Not used. */ )
	{
		switch (reason)
		{
		case DLL_PROCESS_ATTACH:
			g_hInstance = hInst;
			ThTypes_Init();
			InstallMuterHooks();
			break;

		case DLL_PROCESS_DETACH:
			UnInstallMuterHooks();
			ThTypes_End();
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