#pragma once
#include <windows.h>
#include <atlbase.h>
#include <atlsync.h>
#include <mmdeviceapi.h>
#include <audiopolicy.h>

#include <shlwapi.h>
#pragma comment(lib, "Shlwapi.lib")

// {F432FAD2-59F3-41d6-ADBF-478D6E12F6B7}
static const GUID AudioVolumnCtx = { 0xf432fad2, 0x59f3, 0x41d6, { 0xad, 0xbf, 0x47, 0x8d, 0x6e, 0x12, 0xf6, 0xb7 } };

class AudioVolume: public IMMNotificationClient, IAudioSessionEvents
{
private:
	BOOL                            m_bRegisteredForEndpointNotifications;
	BOOL                            m_bRegisteredForAudioSessionNotifications;
	CComPtr<IMMDeviceEnumerator>    m_spEnumerator;
	CComPtr<IMMDevice>              m_spAudioEndpoint;
	CComPtr<IAudioSessionManager>   m_spAudioSessionManager;
	CComPtr<IAudioSessionControl>   m_spAudioSessionControl;
	CComPtr<ISimpleAudioVolume>     m_spSimpleAudioVolume;

	CCriticalSection                m_csEndpoint;

	long                            m_cRef;

	~AudioVolume();       // refcounted object... make the destructor private

	HRESULT AttachToDefaultEndpoint();
	void    DetachFromEndpoint();

	// IMMNotificationClient (only need to really implement OnDefaultDeviceChanged)
	IFACEMETHODIMP OnDeviceStateChanged(LPCWSTR pwstrDeviceId, DWORD dwNewState)    {   return S_OK;    }
	IFACEMETHODIMP OnDeviceAdded(LPCWSTR pwstrDeviceId)   {   return S_OK;    }
	IFACEMETHODIMP OnDeviceRemoved(LPCWSTR pwstrDeviceId) {   return S_OK;    }
	IFACEMETHODIMP OnDefaultDeviceChanged(EDataFlow flow, ERole role, LPCWSTR pwstrDefaultDeviceId);
	IFACEMETHODIMP OnPropertyValueChanged(LPCWSTR pwstrDeviceId, const PROPERTYKEY key)   {   return S_OK;    }
	IFACEMETHODIMP OnDeviceQueryRemove()   {   return S_OK;    }
	IFACEMETHODIMP OnDeviceQueryRemoveFailed() {   return S_OK;    }
	IFACEMETHODIMP OnDeviceRemovePending() {   return S_OK;    }

	// IAudioSessionEvents (only need to really implement OnSimpleVolumeChanged)
	IFACEMETHODIMP OnDisplayNameChanged(LPCWSTR NewDisplayName, LPCGUID EventContext) {return S_OK;}
	IFACEMETHODIMP OnIconPathChanged(LPCWSTR NewIconPath, LPCGUID EventContext){return S_OK;}
	IFACEMETHODIMP OnSimpleVolumeChanged(float NewVolume, BOOL NewMute, LPCGUID EventContext);
	IFACEMETHODIMP OnChannelVolumeChanged(DWORD ChannelCount, float NewChannelVolumeArray[], DWORD ChangedChannel, LPCGUID EventContext) {return S_OK;}
	IFACEMETHODIMP OnGroupingParamChanged(LPCGUID NewGroupingParam,  LPCGUID EventContext) {return S_OK;}
	IFACEMETHODIMP OnStateChanged(AudioSessionState NewState){return S_OK;}
	IFACEMETHODIMP OnSessionDisconnected(AudioSessionDisconnectReason DisconnectReason){return S_OK;}

	// IUnknown
	IFACEMETHODIMP QueryInterface(const IID& iid, void** ppUnk);
public:
	AudioVolume();

	HRESULT Initialize();
	void    Dispose();

	void EnableMute(BOOL bEnabled);

	// IUnknown
	IFACEMETHODIMP_(ULONG) AddRef();
	IFACEMETHODIMP_(ULONG) Release();
};
