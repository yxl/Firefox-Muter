#pragma once
#include "dsound.h"

typedef MMRESULT (WINAPI *waveOutWrite_t)(HWAVEOUT hwo, LPWAVEHDR pwh, UINT cbwh);
MMRESULT WINAPI waveOutWrite_hook(HWAVEOUT hwo, LPWAVEHDR pwh, UINT cbwh);

typedef BOOL (WINAPI *midiStreamOut_t)(HMIDISTRM hms, LPMIDIHDR pmh, UINT cbmh);
MMRESULT WINAPI midiStreamOut_hook(HMIDISTRM hms, LPMIDIHDR pmh, UINT cbmh);

typedef int (WINAPI* DirectSoundCreate_t)(LPCGUID pcGuidDevice, 
										  LPDIRECTSOUND *ppDS, 
										  LPUNKNOWN pUnkOuter);
HRESULT WINAPI DirectSoundCreate_hook(LPCGUID pcGuidDevice, 
									  LPDIRECTSOUND *ppDS, 
									  LPUNKNOWN pUnkOuter);

typedef int (WINAPI* DirectSoundCreate8_t)(LPCGUID pcGuidDevice, 
										   LPDIRECTSOUND8 *ppDS, 
										   LPUNKNOWN pUnkOuter);

HRESULT WINAPI DirectSoundCreate8_hook(LPCGUID pcGuidDevice, 
									   LPDIRECTSOUND8 *ppDS, 
									   LPUNKNOWN pUnkOuter);