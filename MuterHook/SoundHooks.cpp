#include "SoundHooks.h"
#include "ThTypes.h"
#include "ApiHooks.h"
#include "DllEntry.h"
#include "HookMgr.h"
#include "HookedDsound.h"

MMRESULT WINAPI waveOutWrite_hook(HWAVEOUT hwo, LPWAVEHDR pwh, UINT cbwh) 
{
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

	waveOutWrite_t waveOutWrite_next = (waveOutWrite_t)g_hookMgr.FindOriginalFunc(hModule, (PROC)waveOutWrite_hook);
	return waveOutWrite_next(hwo, pwh, cbwh);
}

MMRESULT WINAPI midiStreamOut_hook(HMIDISTRM hms, LPMIDIHDR pmh, UINT cbmh) 
{
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

	midiStreamOut_t midiStreamOut_next = (midiStreamOut_t)g_hookMgr.FindOriginalFunc(hModule, (PROC)midiStreamOut_hook);
	return midiStreamOut_next(hms, pmh, cbmh);
}

HRESULT WINAPI DirectSoundCreate_hook(LPCGUID pcGuidDevice, 
									  LPDIRECTSOUND *ppDS, 
									  LPUNKNOWN pUnkOuter) 
{
	DWORD dwCaller;
	GET_CALLER(dwCaller);
	HMODULE hModule = ModuleFromAddress((PVOID)dwCaller);

	DirectSoundCreate_t DirectSoundCreate_next = (DirectSoundCreate_t)g_hookMgr.FindOriginalFunc(hModule, (PROC)DirectSoundCreate_hook);

	HRESULT hr = DirectSoundCreate_next(pcGuidDevice, ppDS, pUnkOuter);

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
	DWORD dwCaller;
	GET_CALLER(dwCaller);
	HMODULE hModule = ModuleFromAddress((PVOID)dwCaller);

	DirectSoundCreate8_t DirectSoundCreate8_next = (DirectSoundCreate8_t)g_hookMgr.FindOriginalFunc(hModule, (PROC)DirectSoundCreate8_hook);

	HRESULT hr = DirectSoundCreate8_next(pcGuidDevice, ppDS, pUnkOuter);

	if (SUCCEEDED(hr)) 
	{
		HookedDirectSound8* p = new HookedDirectSound8;
		p->direct_sound_ = *ppDS;
		*ppDS = p;
	}
	return hr;
}
