#pragma once

extern MMRESULT (WINAPI *waveOutWrite_original)(HWAVEOUT hwo, LPWAVEHDR pwh, UINT cbwh);

MMRESULT WINAPI waveOutWrite_hook(HWAVEOUT hwo, LPWAVEHDR pwh, UINT cbwh);

extern MMRESULT (WINAPI *waveOutOpen_original)(LPHWAVEOUT phwo,
							UINT uDeviceID,
							LPCWAVEFORMATEX pwfx,
							DWORD_PTR dwCallback,
							DWORD_PTR dwInstance,
							DWORD fdwOpen);

MMRESULT WINAPI waveOutOpen_hook(LPHWAVEOUT phwo,
							UINT uDeviceID,
							LPCWAVEFORMATEX pwfx,
							DWORD_PTR dwCallback,
							DWORD_PTR dwInstance,
							DWORD fdwOpen);

extern MMRESULT (WINAPI *waveOutClose_original)(HWAVEOUT hwo);

MMRESULT WINAPI waveOutClose_hook(HWAVEOUT hwo);

extern MMRESULT (WINAPI *waveOutGetVolume_original)(HWAVEOUT hwo, LPDWORD pdwVolume);

MMRESULT WINAPI waveOutGetVolume_hook(HWAVEOUT hwo, LPDWORD pdwVolume);

extern MMRESULT (WINAPI *waveOutSetVolume_original)(HWAVEOUT hwo, DWORD dwVolume);

MMRESULT WINAPI waveOutSetVolume_hook(HWAVEOUT hwo, DWORD dwVolume);


extern HRESULT (WINAPI* DirectSoundCreate_original)(LPCGUID pcGuidDevice, 
										  LPDIRECTSOUND *ppDS, 
										  LPUNKNOWN pUnkOuter);

HRESULT WINAPI DirectSoundCreate_hook(LPCGUID pcGuidDevice, 
									  LPDIRECTSOUND *ppDS, 
									  LPUNKNOWN pUnkOuter);

extern HRESULT (WINAPI* DirectSoundCreate8_original)(LPCGUID pcGuidDevice, 
										   LPDIRECTSOUND8 *ppDS, 
										   LPUNKNOWN pUnkOuter);

HRESULT WINAPI DirectSoundCreate8_hook(LPCGUID pcGuidDevice, 
									   LPDIRECTSOUND8 *ppDS, 
									   LPUNKNOWN pUnkOuter);