#pragma once

#include "ApiHooks.h"

#define MY_DEFINE_GUID(name, l, w1, w2, b1, b2, b3, b4, b5, b6, b7, b8) \
        EXTERN_C const GUID DECLSPEC_SELECTANY name \
                = { l, w1, w2, { b1, b2,  b3,  b4,  b5,  b6,  b7,  b8 } }

MY_DEFINE_GUID(IID_IDirectSound, 0x279AFA83, 0x4981, 0x11CE, 0xA5, 0x21, 0x00, 0x20, 0xAF, 0x0B, 0xE5, 0x60);

MY_DEFINE_GUID(IID_IDirectSound8, 0xC50A7E93, 0xF395, 0x4834, 0x9E, 0xF6, 0x7F, 0xA9, 0x9D, 0xE5, 0x09, 0x66);

/**
* The original code comes from  Chrome Toolbox project(http://code.google.com/p/chrome-toolbox/)
*/

// it's a wapper of IDirectSoundBuffer interface for mute some audio
// played by dsound
class HookedDirectSoundBuffer : public IDirectSoundBuffer {
	// IUnknown methods
	STDMETHOD(QueryInterface)       (THIS_ __in REFIID iid, __deref_out LPVOID* lpout) {
		return directsound_buffer_->QueryInterface(iid, lpout);
	}
	STDMETHOD_(ULONG,AddRef)        (THIS) {
		return directsound_buffer_->AddRef();
	}
	STDMETHOD_(ULONG,Release)       (THIS) {
		ULONG cnt = directsound_buffer_->Release();
		if (cnt == 0) {
			delete this;
		}
		return cnt;
	}

	// IDirectSoundBuffer methods
	STDMETHOD(GetCaps)              (THIS_ __out LPDSBCAPS pDSBufferCaps) {
		return directsound_buffer_->GetCaps(pDSBufferCaps);
	}
	STDMETHOD(GetCurrentPosition)   (THIS_ __out_opt LPDWORD pdwCurrentPlayCursor, __out_opt LPDWORD pdwCurrentWriteCursor) {
		return directsound_buffer_->GetCurrentPosition(pdwCurrentPlayCursor, pdwCurrentWriteCursor);
	}
	STDMETHOD(GetFormat)            (THIS_ __out_bcount_opt(dwSizeAllocated) LPWAVEFORMATEX pwfxFormat, DWORD dwSizeAllocated, __out_opt LPDWORD pdwSizeWritten) {
		return directsound_buffer_->GetFormat(pwfxFormat, dwSizeAllocated, pdwSizeWritten);
	}
	STDMETHOD(GetVolume)            (THIS_ __out LPLONG plVolume) {
		HRESULT hr = directsound_buffer_->GetVolume(plVolume);
		if (hr == S_OK && plVolume) {
			*plVolume = mVolume;
		}
		return hr;
	}
	STDMETHOD(GetPan)               (THIS_ __out LPLONG plPan) {
		return directsound_buffer_->GetPan(plPan);
	}
	STDMETHOD(GetFrequency)         (THIS_ __out LPDWORD pdwFrequency) {
		return directsound_buffer_->GetFrequency(pdwFrequency);
	}
	STDMETHOD(GetStatus)            (THIS_ __out LPDWORD pdwStatus) {
		return directsound_buffer_->GetStatus(pdwStatus);
	}
	STDMETHOD(Initialize)           (THIS_ __in LPDIRECTSOUND pDirectSound, __in LPCDSBUFFERDESC pcDSBufferDesc) {
		return directsound_buffer_->Initialize(pDirectSound, pcDSBufferDesc);
	}
	STDMETHOD(Lock)                 (THIS_ DWORD dwOffset, DWORD dwBytes,
		__deref_out_bcount(*pdwAudioBytes1) LPVOID *ppvAudioPtr1, __out LPDWORD pdwAudioBytes1,
		__deref_opt_out_bcount(*pdwAudioBytes2) LPVOID *ppvAudioPtr2, __out_opt LPDWORD pdwAudioBytes2, DWORD dwFlags) {
		return directsound_buffer_->Lock(dwOffset, dwBytes, ppvAudioPtr1, pdwAudioBytes1, ppvAudioPtr2, pdwAudioBytes2, dwFlags);
	}
	STDMETHOD(Play)                 (THIS_ DWORD dwReserved1, DWORD dwPriority, DWORD dwFlags) {
		return directsound_buffer_->Play(dwReserved1, dwPriority, dwFlags);
	}
	STDMETHOD(SetCurrentPosition)   (THIS_ DWORD dwNewPosition) {
		return directsound_buffer_->SetCurrentPosition(dwNewPosition);
	}
	STDMETHOD(SetFormat)            (THIS_ __in LPCWAVEFORMATEX pcfxFormat) {
		return directsound_buffer_->SetFormat(pcfxFormat);
	}
	STDMETHOD(SetVolume)            (THIS_ LONG lVolume) {
		mVolume = lVolume;
		return directsound_buffer_->SetVolume(GetTargetVolume());
	}
	STDMETHOD(SetPan)               (THIS_ LONG lPan) {
		return directsound_buffer_->SetPan(lPan);
	}
	STDMETHOD(SetFrequency)         (THIS_ DWORD dwFrequency) {
		return directsound_buffer_->SetFrequency(dwFrequency);
	}
	STDMETHOD(Stop)                 (THIS) {
		return directsound_buffer_->Stop();
	}
	STDMETHOD(Unlock)               (THIS_ __in_bcount(dwAudioBytes1) LPVOID pvAudioPtr1, DWORD dwAudioBytes1,
		__in_bcount_opt(dwAudioBytes2) LPVOID pvAudioPtr2, DWORD dwAudioBytes2) {
		UpdateVolume();

		return directsound_buffer_->Unlock(pvAudioPtr1, dwAudioBytes1, pvAudioPtr2, dwAudioBytes2);
	}
	STDMETHOD(Restore)              (THIS) {
		return directsound_buffer_->Restore();
	}

public:
	HookedDirectSoundBuffer(IDirectSoundBuffer* pBuffer) {
		directsound_buffer_ = pBuffer;
		mVolume = 0;
		pBuffer->GetVolume(&mVolume);
	}
private:
	void UpdateVolume() {
		LONG actualVolume = DSBVOLUME_MAX;
		if (directsound_buffer_->GetVolume(&actualVolume) == S_OK)
		{
			LONG targetVolume = GetTargetVolume();
			if (actualVolume != targetVolume) {
				directsound_buffer_->SetVolume(targetVolume);
			}
		}
	}

	LONG GetTargetVolume() {
		if (IsMuteEnabled())
		{
			return DSBVOLUME_MIN;
		}
		return DSBVOLUME_MIN +  (mVolume - DSBVOLUME_MIN) * ::GetVolume() / MAX_VOLUME;
	}

	IDirectSoundBuffer* directsound_buffer_;
	LONG mVolume;
};

class HookedDirectSound8 : public IDirectSound8 {
	// IUnknown methods
	STDMETHOD(QueryInterface)       (THIS_ __in REFIID iid, __deref_out LPVOID* lpout) {
		HRESULT hr = direct_sound_->QueryInterface(iid, lpout);
		if (IsEqualIID(iid, IID_IDirectSound) || IsEqualIID(iid, IID_IDirectSound8)) {
			*lpout = static_cast<LPVOID>(this);
		}
		return hr;
	}
	STDMETHOD_(ULONG,AddRef)        (THIS) {
		return direct_sound_->AddRef();
	}
	STDMETHOD_(ULONG,Release)       (THIS) {
		ULONG cnt = direct_sound_->Release();
		if (cnt == 0) {
			delete this;
		}
		return cnt;
	}

	// IDirectSound methods
	STDMETHOD(CreateSoundBuffer)    (THIS_ __in LPCDSBUFFERDESC pcDSBufferDesc, __deref_out LPDIRECTSOUNDBUFFER *ppDSBuffer, __null LPUNKNOWN pUnkOuter) {
		HRESULT hr = direct_sound_->CreateSoundBuffer(pcDSBufferDesc, ppDSBuffer, pUnkOuter);
		if (SUCCEEDED(hr)) {
			HookedDirectSoundBuffer* p = new HookedDirectSoundBuffer(*ppDSBuffer);
			*ppDSBuffer = p;
		}
		return hr;
	}
	STDMETHOD(GetCaps)              (THIS_ __out LPDSCAPS pDSCaps) {
		return direct_sound_->GetCaps(pDSCaps);
	}
	STDMETHOD(DuplicateSoundBuffer) (THIS_ __in LPDIRECTSOUNDBUFFER pDSBufferOriginal, __deref_out LPDIRECTSOUNDBUFFER *ppDSBufferDuplicate) {
		return direct_sound_->DuplicateSoundBuffer(pDSBufferOriginal, ppDSBufferDuplicate);
	}
	STDMETHOD(SetCooperativeLevel)  (THIS_ HWND hwnd, DWORD dwLevel) {
		return direct_sound_->SetCooperativeLevel(hwnd, dwLevel);
	}
	STDMETHOD(Compact)              (THIS) {
		return direct_sound_->Compact();
	}
	STDMETHOD(GetSpeakerConfig)     (THIS_ __out LPDWORD pdwSpeakerConfig) {
		return direct_sound_->GetSpeakerConfig(pdwSpeakerConfig);
	}
	STDMETHOD(SetSpeakerConfig)     (THIS_ DWORD dwSpeakerConfig) {
		return direct_sound_->SetSpeakerConfig(dwSpeakerConfig);
	}
	STDMETHOD(Initialize)           (THIS_ __in_opt LPCGUID pcGuidDevice) {
		return direct_sound_->Initialize(pcGuidDevice);
	}

	// IDirectSound8 methods
	STDMETHOD(VerifyCertification)  (THIS_ __out LPDWORD pdwCertified) {
		return direct_sound_->VerifyCertification(pdwCertified);
	}

public:
	HookedDirectSound8(IDirectSound8* pSound) {
		direct_sound_ = pSound;
	}
	HookedDirectSound8(IDirectSound* pSound) {
		direct_sound_ = static_cast<IDirectSound8*>(pSound);
	}
private:
	IDirectSound8* direct_sound_;
};