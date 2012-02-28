// MuterWin7.cpp : Defines the exported functions for the DLL application.
//

#include "stdafx.h"
#include "MuterWin7.h"
#include "AudioVolume.h"
#include "SDKTrace.h"

#include <iostream>
#include <process.h>

#ifdef __cplusplus
extern "C"
{
#endif				/* __cplusplus */

	AudioVolume* g_pAudioVolume = NULL;
	// Thread ID
	unsigned g_uThread = 0;
	// Thread handle
	HANDLE g_hThread;

	unsigned __stdcall ThreadProc(void* lParam);

	MUTERWIN7_API BOOL Initialize(void)
	{
		// Check if it is already initialized.
		if (g_uThread != 0)
		{
			return TRUE;
		}

		// Start thread
		g_hThread = (HANDLE)_beginthreadex( NULL, 0, &ThreadProc, NULL, 0, &g_uThread);
		if(g_hThread == NULL)
		{
			return FALSE;
		}

		return TRUE;
	}

	MUTERWIN7_API void Dispose(void)
	{
		if (g_hThread == NULL)
		{
			return;
		}
		PostThreadMessage(g_uThread, WM_QUIT, 0, 0);
		WaitForSingleObject(g_hThread, INFINITE);
		CloseHandle(g_hThread);
	}

	MUTERWIN7_API void EnableMute(BOOL bEnabled)
	{
		if (g_hThread == NULL)
		{
			return;
		}
		PostThreadMessage(g_uThread, MSG_USER_ENABLE_MUTE, (WPARAM )bEnabled, 0);
	}

	MUTERWIN7_API BOOL IsMuteEnabled()
	{
		if (g_pAudioVolume != NULL)
		{
			return g_pAudioVolume->IsMuted();
		}
		return FALSE;
	}

	unsigned __stdcall ThreadProc(void* lParam)
	{
		try
		{
			HRESULT hr = CoInitializeEx(NULL, COINIT_MULTITHREADED);
			if (FAILED(hr))
			{
				throw "CoInitializeEx failed!";
			}
			g_pAudioVolume = new (std::nothrow)AudioVolume();
			if (g_pAudioVolume == NULL) 
			{
				CoUninitialize();
				throw "new AudioVolume() failed!";
			}
			g_pAudioVolume->Initialize();
			g_pAudioVolume->SetMuteStatus(FALSE);
		}
		catch (LPCSTR szError)
		{
			TRACE("[MuterWin7] ThreadProc: %s\n", szError);
			_endthreadex(1);
		}

		MSG msg;
		while(GetMessage(&msg, 0, 0, 0))
		{
			switch (msg.message)
			{
			case MSG_USER_DEFAULT_DEVICE_CHANGE:
				{
					g_pAudioVolume->UpdateDevice();
				}
				break;
			case MSG_USER_ENABLE_MUTE:
				{
					BOOL bEnabled = (BOOL)msg.wParam;
					g_pAudioVolume->SetMuteStatus(bEnabled);
				}
				break;
			}
			DispatchMessage(&msg);
		}

		g_pAudioVolume->SetMuteStatus(FALSE);
		g_pAudioVolume->Dispose();
		g_pAudioVolume->Release();
		CoUninitialize();

		_endthreadex(0);
		return 0;
	}
#ifdef __cplusplus
}
#endif				/* __cplusplus */
