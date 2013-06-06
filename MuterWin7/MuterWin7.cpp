// MuterWin7.cpp : Defines the exported functions for the DLL application.
//

#include "stdafx.h"
#include "MuterWin7.h"
#include "AudioVolume.h"

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
	// Whether we enable muting
	BOOL g_bEnableMute = FALSE;

	int g_iVolume = MAX_VOLUME;

	const int MIN_VOLUME = 0;
	const int MAX_VOLUME = 100;

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
		g_hThread = NULL;
		g_uThread = 0;
	}

	MUTERWIN7_API void EnableMute(BOOL bEnabled)
	{
		if (g_hThread == NULL)
		{
			return;
		}
		g_bEnableMute = bEnabled;
		PostThreadMessage(g_uThread, MSG_USER_UPDATE_MUTE_STATUS, 0, 0);
	}

	MUTERWIN7_API BOOL IsMuteEnabled()
	{
		return g_bEnableMute;
	}

	/**
	 * Max volume - 100
	 * Min volume - 0
	 */
	MUTERWIN7_API int GetVolume()
	{
		return g_iVolume;
	}

	MUTERWIN7_API void SetVolume(int iVolume)
	{
		if (iVolume < MIN_VOLUME)
		{
			iVolume = MIN_VOLUME;
		}
		if (iVolume > MAX_VOLUME)
		{
			iVolume = MAX_VOLUME;
		}
		g_iVolume = iVolume;
		PostThreadMessage(g_uThread, MSG_USER_UPDATE_MUTE_STATUS, 0, 0);
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
			g_bEnableMute = FALSE;
			g_pAudioVolume->UpdateMuteStatus();
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
					Sleep(200);
					g_pAudioVolume->UpdateDevice();
				}
				break;
			case MSG_USER_UPDATE_MUTE_STATUS:
				{
					g_pAudioVolume->UpdateMuteStatus();
				}
				break;
			}
			DispatchMessage(&msg);
		}

		g_bEnableMute = FALSE;
		g_pAudioVolume->UpdateMuteStatus();
		g_pAudioVolume->Dispose();
		g_pAudioVolume->Release();
		CoUninitialize();

		_endthreadex(0);
		return 0;
	}
#ifdef __cplusplus
}
#endif				/* __cplusplus */
