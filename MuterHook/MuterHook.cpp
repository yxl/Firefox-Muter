#include "MuterHook.h"
#include "ThTypes.h"
#include "ApiHooks.h"

HINSTANCE g_hInstance = NULL;
HANDLE g_hSyncMutex = 0;

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


BOOL APIENTRY DllMain (HINSTANCE hInst     /* Library instance handle. */ ,
                       DWORD reason        /* Reason this function is being called. */ ,
                       LPVOID reserved     /* Not used. */ )
{
    switch (reason)
    {
	case DLL_PROCESS_ATTACH:
			g_hInstance = hInst;
			g_hSyncMutex = CreateMutex(NULL, FALSE, "muter@yxl.name");
			ThTypes_Init();
			InstallMuterHooks();
        break;

      case DLL_PROCESS_DETACH:
			WaitForSingleObject(g_hSyncMutex, INFINITE);
			ReleaseMutex(g_hSyncMutex);
			CloseHandle(g_hSyncMutex);
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
