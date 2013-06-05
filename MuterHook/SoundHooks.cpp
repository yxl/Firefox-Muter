#include "stdafx.h"
#include "SoundHooks.h"
#include "ApiHooks.h"
#include "DllEntry.h"
#include "HookedDsound.h"

// Map saving the original wave volume
std::map<HWAVEOUT, DWORD> s_waveMap;
CCriticalSection s_csWaveMap;

void UpdateWaveVolume(HWAVEOUT hwo);
DWORD GetTargetWaveVolume(HWAVEOUT hwo);

MMRESULT (WINAPI *waveOutWrite_original)(HWAVEOUT hwo, LPWAVEHDR pwh, UINT cbwh) = waveOutWrite;

MMRESULT WINAPI waveOutWrite_hook(HWAVEOUT hwo, LPWAVEHDR pwh, UINT cbwh) 
{
	TRACE("[MuterHook] waveOutWrite_hook\n");

	UpdateWaveVolume(hwo);

	return waveOutWrite_original(hwo, pwh, cbwh);
}

MMRESULT (WINAPI *waveOutOpen_original)(LPHWAVEOUT phwo,
							UINT uDeviceID,
							LPCWAVEFORMATEX pwfx,
							DWORD_PTR dwCallback,
							DWORD_PTR dwInstance,
							DWORD fdwOpen) = waveOutOpen;

MMRESULT WINAPI waveOutOpen_hook(LPHWAVEOUT phwo,
							UINT uDeviceID,
							LPCWAVEFORMATEX pwfx,
							DWORD_PTR dwCallback,
							DWORD_PTR dwInstance,
							DWORD fdwOpen)
{
	TRACE("[MuterHook] waveOutOpen_hook\n");
	MMRESULT ret = waveOutOpen_original(phwo, uDeviceID, pwfx, dwCallback, dwInstance, fdwOpen);
	if (ret == MMSYSERR_NOERROR && phwo)
	{
		DWORD volume = 0xFFFF;
		if (waveOutGetVolume_original(*phwo, &volume) == MMSYSERR_NOERROR)
		{
			s_csWaveMap.Enter();
			s_waveMap.insert(std::make_pair(*phwo, volume));
			s_csWaveMap.Leave();
			TRACE("[MuterHook] %u\n", volume);
		}
	}
	return ret;
}

MMRESULT (WINAPI *waveOutClose_original)(HWAVEOUT hwo) = waveOutClose;

MMRESULT WINAPI waveOutClose_hook(HWAVEOUT hwo)
{
	MMRESULT ret = waveOutClose_original(hwo);
	if (ret == MMSYSERR_NOERROR || ret == MMSYSERR_INVALHANDLE)
	{
		s_csWaveMap.Enter();
		s_waveMap.erase(hwo);
		s_csWaveMap.Leave();
	}
	return ret;
}

MMRESULT (WINAPI *waveOutGetVolume_original)(HWAVEOUT hwo, LPDWORD pdwVolume) = waveOutGetVolume;

MMRESULT WINAPI waveOutGetVolume_hook(HWAVEOUT hwo, LPDWORD pdwVolume)
{
	MMRESULT ret = waveOutGetVolume_original(hwo, pdwVolume);
	if (ret == MMSYSERR_NOERROR && pdwVolume)
	{
		s_csWaveMap.Enter();
		auto iter = s_waveMap.find(hwo);
		if (iter == s_waveMap.end())
		{
			s_waveMap.insert(std::make_pair(hwo, *pdwVolume));
		}
		else
		{
			 *pdwVolume = iter->second;
		}
		s_csWaveMap.Leave();
	}
	return ret;
}

MMRESULT (WINAPI *waveOutSetVolume_original)(HWAVEOUT hwo, DWORD dwVolume) = waveOutSetVolume;

MMRESULT WINAPI waveOutSetVolume_hook(HWAVEOUT hwo, DWORD dwVolume)
{
	s_csWaveMap.Enter();
	auto iter = s_waveMap.find(hwo);
	if (iter == s_waveMap.end())
	{
		s_waveMap.insert(std::make_pair(hwo, dwVolume));
	}
	else
	{
		 iter->second = dwVolume;
	}
	s_csWaveMap.Leave();
	return waveOutSetVolume_original(hwo, GetTargetWaveVolume(hwo));
}

DWORD GetTargetWaveVolume(HWAVEOUT hwo)
{ 
	if (IsMuteEnabled())
	{
		return 0x0000;
	}
	DWORD volume = 0xFFFF;
	s_csWaveMap.Enter();
	auto iter = s_waveMap.find(hwo);
	if (iter == s_waveMap.end())
	{
		s_csWaveMap.Leave();
		if (waveOutGetVolume_original(hwo, &volume) == MMSYSERR_NOERROR)
		{
			s_csWaveMap.Enter();
			s_waveMap.insert(std::make_pair(hwo, volume));
			s_csWaveMap.Leave();
		}
	}
	else
	{
		volume = iter->second;
		s_csWaveMap.Leave();
	}
	return volume * ::GetVolume() / MAX_VOLUME;
}

void UpdateWaveVolume(HWAVEOUT hwo)
{
	DWORD actualVolume = 0xFFFF;
	DWORD targetVolume = GetTargetWaveVolume(hwo);
	if (MMSYSERR_NOERROR == waveOutGetVolume_original(hwo, &actualVolume) &&
		actualVolume != targetVolume)
	{
		waveOutSetVolume_original(hwo, targetVolume);
	}
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

	if (SUCCEEDED(hr)) 
	{
		*ppDS = new HookedDirectSound8(*ppDS);
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

	if (SUCCEEDED(hr))
	{
		*ppDS = new HookedDirectSound8(*ppDS);
	}
	return hr;
}
