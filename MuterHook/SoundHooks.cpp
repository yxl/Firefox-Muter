#include "SoundHooks.h"
#include "ApiHooks.h"
#include "DllEntry.h"
#include "HookedDsound.h"
#include "SDKTrace.h"
#pragma comment(lib, "winmm.lib")

MMRESULT (WINAPI *waveOutWrite_original)(HWAVEOUT hwo, LPWAVEHDR pwh, UINT cbwh) = NULL;

MMRESULT WINAPI waveOutWrite_hook(HWAVEOUT hwo, LPWAVEHDR pwh, UINT cbwh) 
{
	TRACE("[MuterHook] waveOutWrite_hook\n");

	if (IsInThisModuleProcess())
	{
		if (ShouldMute())
		{ 
			memset(pwh->lpData, 0 , pwh->dwBufferLength);
		}
	}

	return waveOutWrite_original(hwo, pwh, cbwh);
}

MMRESULT (WINAPI *midiStreamOut_original)(HMIDISTRM hms, LPMIDIHDR pmh, UINT cbmh) = NULL;

MMRESULT WINAPI midiStreamOut_hook(HMIDISTRM hms, LPMIDIHDR pmh, UINT cbmh) 
{
	TRACE("[MuterHook] midiStreamOut_hook\n");

	if (IsInThisModuleProcess())
	{
		if (ShouldMute()) 
		{
			memset(pmh->lpData, 0 , pmh->dwBufferLength);
		}	
	}

	return midiStreamOut_original(hms, pmh, cbmh);
}

HRESULT (WINAPI* DirectSoundCreate_original)(LPCGUID pcGuidDevice, 
											 LPDIRECTSOUND *ppDS, 
											 LPUNKNOWN pUnkOuter) =  NULL;

HRESULT WINAPI DirectSoundCreate_hook(LPCGUID pcGuidDevice, 
									  LPDIRECTSOUND *ppDS, 
									  LPUNKNOWN pUnkOuter) 
{
	TRACE("[MuterHook] DirectSoundCreate_hook\n");

	HRESULT hr = DirectSoundCreate_original(pcGuidDevice, ppDS, pUnkOuter);

	if (SUCCEEDED(hr) && IsInThisModuleProcess()) 
	{
		HookedDirectSound* p = new HookedDirectSound;
		p->direct_sound_ = *ppDS;
		*ppDS = p;

	}
	return hr;
}

HRESULT (WINAPI* DirectSoundCreate8_original)(LPCGUID pcGuidDevice, 
											  LPDIRECTSOUND8 *ppDS, 
											  LPUNKNOWN pUnkOuter) = NULL;

HRESULT WINAPI DirectSoundCreate8_hook(LPCGUID pcGuidDevice, 
									   LPDIRECTSOUND8 *ppDS, 
									   LPUNKNOWN pUnkOuter) 
{
	TRACE("[MuterHook] DirectSoundCreate8_hook\n");

	HRESULT hr = DirectSoundCreate8_original(pcGuidDevice, ppDS, pUnkOuter);

	if (SUCCEEDED(hr) && IsInThisModuleProcess()) 
	{
		HookedDirectSound8* p = new HookedDirectSound8;
		p->direct_sound_ = *ppDS;
		*ppDS = p;
	}
	return hr;
}
