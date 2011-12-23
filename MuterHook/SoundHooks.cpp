#include "SoundHooks.h"
#include "ThTypes.h"
#include "ApiHooks.h"
#include "DllEntry.h"
#include "HookMgr.h"
#include "HookedDsound.h"
#include "SDKTrace.h"

#pragma comment(lib, "Winmm.lib")
#pragma comment(lib, "DSound.lib")

MMRESULT WINAPI waveOutWrite_hook(HWAVEOUT hwo, LPWAVEHDR pwh, UINT cbwh) 
{
  TRACE("[MuterHook] waveOutWrite_hook\n");
	DWORD dwCaller;
	GET_CALLER(dwCaller);
	HMODULE hModule = ModuleFromAddress((PVOID)dwCaller);

	if (GlobalData)
	{
		GlobalData->lLastSoundPlayingTimeInSeconds = (LONG)time(NULL);
	}
	if (GlobalData && GlobalData->bMute) 
	{ 
		memset(pwh->lpData, 0 , pwh->dwBufferLength);
	}

	return ::waveOutWrite(hwo, pwh, cbwh);
}

MMRESULT WINAPI midiStreamOut_hook(HMIDISTRM hms, LPMIDIHDR pmh, UINT cbmh) 
{
  TRACE("[MuterHook] midiStreamOut_hook\n");
	DWORD dwCaller;
	GET_CALLER(dwCaller);
	HMODULE hModule = ModuleFromAddress((PVOID)dwCaller);

	if (GlobalData)
	{
		GlobalData->lLastSoundPlayingTimeInSeconds = (LONG)time(NULL);
	}
	if (GlobalData && GlobalData->bMute) 
	{
		memset(pmh->lpData, 0 , pmh->dwBufferLength);
	}

	::midiStreamOut(hms, pmh, cbmh);
}

HRESULT WINAPI DirectSoundCreate_hook(LPCGUID pcGuidDevice, 
									  LPDIRECTSOUND *ppDS, 
									  LPUNKNOWN pUnkOuter) 
{
  TRACE("[MuterHook] DirectSoundCreate_hook\n");
	DWORD dwCaller;
	GET_CALLER(dwCaller);
	HMODULE hModule = ModuleFromAddress((PVOID)dwCaller);

	HRESULT hr = ::DirectSoundCreate(pcGuidDevice, ppDS, pUnkOuter);

	if (SUCCEEDED(hr)) 
	{
		HookedDirectSound* p = new HookedDirectSound;
		p->direct_sound_ = *ppDS;
		*ppDS = p;

	}
	return hr;
}

HRESULT WINAPI DirectSoundCreate8_hook(LPCGUID pcGuidDevice, 
									   LPDIRECTSOUND8 *ppDS, 
									   LPUNKNOWN pUnkOuter) 
{
  TRACE("[MuterHook] DirectSoundCreate8_hook\n");
	DWORD dwCaller;
	GET_CALLER(dwCaller);
	HMODULE hModule = ModuleFromAddress((PVOID)dwCaller);

	HRESULT hr = ::DirectSoundCreate8(pcGuidDevice, ppDS, pUnkOuter);

	if (SUCCEEDED(hr)) 
	{
		HookedDirectSound8* p = new HookedDirectSound8;
		p->direct_sound_ = *ppDS;
		*ppDS = p;
	}
	return hr;
}
