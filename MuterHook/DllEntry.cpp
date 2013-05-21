#include "DllEntry.h"
#include "ApiHooks.h"
#include "SDKTrace.h"
#include "QQMusicHook.h"

#include <process.h>

#ifdef __cplusplus
extern "C"
{
#endif				/* __cplusplus */

	// Custom thread message ID
	const UINT WM_USER_HOOK_PROCESSES = WM_USER + 200;

	CHAR g_szThisModulePath[MAX_PATH];

	// Thread handle
	HANDLE g_hThread;

	/**
	* Shared by all processes variables.
	*/
#pragma data_seg(".HKT")
	BOOL g_bMuted = FALSE;
	// Thread ID
	unsigned g_uThread = 0;
#pragma data_seg()
#pragma comment(linker, "/Section:.HKT,rws")  


	/*DLLIMPORT*/ void EnableMute(BOOL bEnabled)
	{
		g_bMuted = bEnabled;
		if (bEnabled)
		{
			InstallMuterHooks();
			PostThreadMessage(g_uThread, WM_USER_HOOK_PROCESSES, 0, 0);
		}
	}

	/*DLLIMPORT*/ BOOL IsMuteEnabled()
	{
		return g_bMuted;
	}

	unsigned __stdcall ThreadProc(void* lParam);

	DLLIMPORT BOOL Initialize(void)
	{
		// Check if it is already initialized.
		if (g_uThread != 0)
		{
			return TRUE;
		}

		g_bMuted = FALSE;

		// Start thread
		g_hThread = reinterpret_cast<HANDLE>(_beginthreadex( NULL, 0, &ThreadProc, NULL, 0, &g_uThread));
		if(g_hThread == NULL)
		{
			return FALSE;
		}

		return TRUE;
	}

	DLLIMPORT void Dispose(void)
	{
		if (g_uThread == 0)
		{
			return;
		}

		g_bMuted = FALSE;

		PostThreadMessage(g_uThread, WM_QUIT, 0, 0);
		WaitForSingleObject(g_hThread, INFINITE);
		CloseHandle(g_hThread);
		g_hThread = NULL;
		g_uThread = 0;
	}

	unsigned __stdcall ThreadProc(void* lParam)
	{
		MSG msg;
		while(GetMessage(&msg, 0, 0, 0))
		{
			switch (msg.message)
			{
			case WM_USER_HOOK_PROCESSES:
				{
					InjectIntoSubProcesses();
					InjectIntoQQMusic();
				}
				break;
			}
			DispatchMessage(&msg);
		}

		_endthreadex(0);
		return 0;
	}

	BOOL APIENTRY DllMain (HINSTANCE hInst     /* Library instance handle. */ ,
		DWORD reason        /* Reason this function is being called. */ ,
		LPVOID reserved     /* Not used. */ )
	{

		switch (reason)
		{
		case DLL_PROCESS_ATTACH:
			GetModuleFileNameA(hInst, g_szThisModulePath, sizeof(g_szThisModulePath) / sizeof(CHAR));
			InstallMuterHooks();
			PostThreadMessage(g_uThread, WM_USER_HOOK_PROCESSES, 0, 0);
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