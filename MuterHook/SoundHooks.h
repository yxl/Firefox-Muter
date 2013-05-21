#pragma once
#define INITGUID
#include "external\dsound.h"

extern MMRESULT (WINAPI *waveOutWrite_original)(HWAVEOUT hwo, LPWAVEHDR pwh, UINT cbwh);

MMRESULT WINAPI waveOutWrite_hook(HWAVEOUT hwo, LPWAVEHDR pwh, UINT cbwh);

extern MMRESULT (WINAPI *midiStreamOut_original)(HMIDISTRM hms, LPMIDIHDR pmh, UINT cbmh);

MMRESULT WINAPI midiStreamOut_hook(HMIDISTRM hms, LPMIDIHDR pmh, UINT cbmh);


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