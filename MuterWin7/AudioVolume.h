#pragma once

// {F432FAD2-59F3-41d6-ADBF-478D6E12F6B7}
static const GUID AudioVolumnCtx = { 0xf432fad2, 0x59f3, 0x41d6, { 0xad, 0xbf, 0x47, 0x8d, 0x6e, 0x12, 0xf6, 0xb7 } };

/**
* The session enumerator maintains a list of current sessions by holding references to each session's 
* IAudioSessionControl pointer. However, the session enumerator might not be aware of the new sessions
* that are reported through IAudioSessionNotification. In that case, the application would have access
* to only a partial list of sessions. This might occur if the IAudioSessionControl pointer (in the 
* callback) is released before the session enumerator is initialized. Therefore, if an application 
* wants a complete set of sessions for the audio endpoint, the application should maintain its own list.
* For more information, see IAudioSessionEnumerator interface at 
* http://msdn.microsoft.com/en-us/library/dd368281(v=VS.85).aspx#1.
*/
class AudioVolume: public IMMNotificationClient, IAudioSessionNotification
{
private:
	BOOL                              m_bRegisteredForEndpointNotifications;
	BOOL                              m_bRegisteredForAudioSessionNotifications;
	CComQIPtr<IMMDeviceEnumerator>    m_spEnumerator;
	CComQIPtr<IMMDevice>              m_spAudioEndpoint;
	CComQIPtr<IAudioSessionManager2>  m_spAudioSessionManager2;

	// Audio Session Control list
	CAtlMap<CStringW, CComQIPtr<IAudioSessionControl> , CElementTraits<CStringW> > m_mapSpAudioSessionControl2;
	CCriticalSection                  m_csEndpoint;

	long                              m_cRef;

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

	// IAudioSessionNotification
	IFACEMETHODIMP OnSessionCreated(IAudioSessionControl *NewSession);

	// IUnknown
	IFACEMETHODIMP QueryInterface(const IID& iid, void** ppUnk);

	// Get a map that its keys contains all process IDs and the value for each key is the subprocess ID.
	BOOL BuildProcesseTree(std::map<DWORD, DWORD> &map);

	BOOL IsQzoneMusicProcess(DWORD processId);
	BOOL IsDescendantProcess(std::map<DWORD, DWORD> &map,  DWORD processId, int depth = 8);

	void InitAudioSessionControlList();
	void DisposeAudioSessionControlList();
	void UpdateAudioSessionControlMuteStatus();

	void AddSession(std::map<DWORD, DWORD> &map, CComQIPtr<IAudioSessionControl> spAudioSessionControl);
public:
	AudioVolume();

	HRESULT Initialize();
	void    Dispose();

	// Update mute status of all the audio sessions
	void UpdateMuteStatus();

	// ----------------------------------------------------------------------
	//  Call this from the main thread when the default device changes
	//
	// ----------------------------------------------------------------------
	void UpdateDevice() 
	{
		m_csEndpoint.Enter();
		DetachFromEndpoint();
		AttachToDefaultEndpoint();
		m_csEndpoint.Leave();
	}

	// IUnknown
	IFACEMETHODIMP_(ULONG) AddRef();
	IFACEMETHODIMP_(ULONG) Release();
};
