#include "stdafx.h"
#include "AudioVolume.h"
#include "MuterWin7.h"
#include "SDKTrace.h"

#include <tlhelp32.h>

#define EXIT_ON_ERROR(hr) \
if (FAILED(hr)) { goto Exit; }

#define SAFE_RELEASE(sp) \
if ((sp) != NULL) \
{ (sp).Release();}

AudioVolume::AudioVolume(void)
	: m_bRegisteredForEndpointNotifications(FALSE)
	, m_bRegisteredForAudioSessionNotifications(FALSE)
	, m_cRef(1)
{
}

AudioVolume::~AudioVolume(void)
{
}

// ----------------------------------------------------------------------
//  Initialize this object.  Call after constructor.
//
// ----------------------------------------------------------------------
HRESULT AudioVolume::Initialize()
{
	HRESULT hr;

	// create enumerator
	hr = m_spEnumerator.CoCreateInstance(__uuidof(MMDeviceEnumerator));
	if (SUCCEEDED(hr))
	{
		hr = m_spEnumerator->RegisterEndpointNotificationCallback(this);
		if (SUCCEEDED(hr))
		{
			hr = AttachToDefaultEndpoint();
		}
	}

	return hr;
}

// ----------------------------------------------------------------------
//  Call when the app is done with this object before calling release.
//  This detaches from the endpoint and releases all audio service references.
//
// ----------------------------------------------------------------------
void AudioVolume::Dispose()
{
	DetachFromEndpoint();

	if (m_bRegisteredForEndpointNotifications)
	{
		m_spEnumerator->UnregisterEndpointNotificationCallback(this);
		m_bRegisteredForEndpointNotifications = FALSE;
	}
}


// ----------------------------------------------------------------------
//  Start monitoring the current default device
//
// ----------------------------------------------------------------------
HRESULT AudioVolume::AttachToDefaultEndpoint()
{
	m_csEndpoint.Enter();

	// get the default music & movies playback device
	HRESULT hr = m_spEnumerator->GetDefaultAudioEndpoint(eRender, eMultimedia, &m_spAudioEndpoint);
	if (SUCCEEDED(hr))
	{
		// Get the session manager for this device.
		hr = m_spAudioEndpoint->Activate(__uuidof(IAudioSessionManager2), CLSCTX_INPROC_SERVER, NULL, (void**)&m_spAudioSessionManager2);
		if (SUCCEEDED(hr))
		{
			hr = m_spAudioSessionManager2->RegisterSessionNotification(this);
			m_bRegisteredForAudioSessionNotifications = SUCCEEDED(hr);
		}
	}

	m_csEndpoint.Leave();

	return hr;
}

// ----------------------------------------------------------------------
//  Stop monitoring the device and release all associated references
//
// ----------------------------------------------------------------------
void AudioVolume::DetachFromEndpoint()
{
	m_csEndpoint.Enter();

	if (m_spAudioSessionManager2 != NULL)
	{
		// be sure to unregister...
		if (m_bRegisteredForAudioSessionNotifications)
		{
			m_spAudioSessionManager2->UnregisterSessionNotification(this);
			m_bRegisteredForAudioSessionNotifications = FALSE;
		}
		m_spAudioSessionManager2.Release();
	}

	SAFE_RELEASE(m_spAudioEndpoint);

	m_csEndpoint.Leave();
}


// ----------------------------------------------------------------------
//  Implementation of IMMNotificationClient::OnDefaultDeviceChanged
//
//  When the user changes the default output device we want to stop monitoring the
//  former default and start monitoring the new default
//
// ----------------------------------------------------------------------
HRESULT AudioVolume::OnDefaultDeviceChanged
(
 EDataFlow   flow, 
 ERole       role, 
 LPCWSTR     pwstrDefaultDeviceId
 )
{
	DetachFromEndpoint();
	return AttachToDefaultEndpoint();;
}

// ----------------------------------------------------------------------
//  Implementation of IAudioSessionEvents::OnSimpleVolumeChanged
//
//  This is called by the audio core when the process changes the volume or 
//  mute state for the endpoint we are monitoring
//
// ----------------------------------------------------------------------
HRESULT AudioVolume::OnSimpleVolumeChanged(float NewVolume, BOOL NewMute, LPCGUID EventContext)
{
	BOOL bMute = IsMuteEnabled();
	if (bMute != NewMute)
	{
		EnableMute(bMute);
	}
	return S_OK;
}

// ----------------------------------------------------------------------
//  Implementation of IAudioSessionNotification::OnSessionCreated
//
//  Notifies the registered processes that the audio session has been created.
//
// ----------------------------------------------------------------------
HRESULT AudioVolume::OnSessionCreated(IAudioSessionControl *NewSession)
{
	TRACE("AudioVolume::OnSessionCreated\n");
	UpdateMuteStatus();
	return S_OK;
}


//  IUnknown methods

HRESULT AudioVolume::QueryInterface(REFIID iid, void** ppUnk)
{
	if ((iid == __uuidof(IUnknown)) ||
		(iid == __uuidof(IMMNotificationClient)))
	{
		*ppUnk = static_cast<IMMNotificationClient*>(this);
	}
	else if (iid == __uuidof(IAudioSessionEvents))
	{
		*ppUnk = static_cast<IAudioSessionEvents*>(this);
	}
	else if (iid == __uuidof(IAudioSessionNotification))
	{
		*ppUnk = static_cast<IAudioSessionNotification*>(this);
	}
	else
	{
		*ppUnk = NULL;
		return E_NOINTERFACE;
	}

	AddRef();
	return S_OK;
}

ULONG AudioVolume::AddRef()
{
	return InterlockedIncrement(&m_cRef);
}

ULONG AudioVolume::Release()
{
	long lRef = InterlockedDecrement(&m_cRef);
	if (lRef == 0)
	{
		delete this;
	}
	return lRef;
}

// Get a map that its keys contains all process IDs and the value for each key is a boolean value indicating whehter the process with the key is a subprocess.
BOOL AudioVolume::GetSubProcesseMap(DWORD dwParentProcessId, std::map<DWORD, BOOL> &map)
{
	HANDLE hSnapShot = ::CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	if (hSnapShot == INVALID_HANDLE_VALUE)
	{
		return FALSE;
	}

	PROCESSENTRY32 procentry  = { sizeof(PROCESSENTRY32) };
	BOOL bContinue = Process32First(hSnapShot, &procentry);
	while( bContinue )
	{
		DWORD dwProcessId = procentry.th32ProcessID;
		BOOL bIsSubProcess = dwParentProcessId == procentry.th32ParentProcessID;
		map.insert(std::make_pair(dwProcessId, bIsSubProcess));
		bContinue = Process32Next( hSnapShot, &procentry );
	}

	CloseHandle(hSnapShot);
	return TRUE;
}

// Change mute status of all audio session
void AudioVolume::UpdateMuteStatus()
{
	TRACE("AudioVolume::UpdateMuteStatus\n");
	if (m_spAudioSessionManager2 == NULL)
	{
		return;
	}

	// Map indicating whether a process belongs to firefox
	std::map<DWORD, BOOL> map;
	if (!GetSubProcesseMap(g_dwThisModuleProcessId, map))
	{
		return;
	}
	map.insert(std::make_pair(g_dwThisModuleProcessId, TRUE));

	BOOL bMute = ::IsMuteEnabled();

	CComQIPtr<IAudioSessionEnumerator> spAudioSessionEnumerator;

	// Get audio session enumerator
	EXIT_ON_ERROR(m_spAudioSessionManager2->GetSessionEnumerator(&spAudioSessionEnumerator));

	// Get audio session number
	int nSessions = 0;
	EXIT_ON_ERROR(spAudioSessionEnumerator->GetCount(&nSessions));

	// Enumerate audio sessions
	for (int i=0; i<nSessions; i++)
	{
		// Get AudioSessionControl2
		CComQIPtr<IAudioSessionControl> spAudioSessionControl;
		EXIT_ON_ERROR(spAudioSessionEnumerator->GetSession(i, &spAudioSessionControl));
		CComQIPtr<IAudioSessionControl2> spAudioSessionControl2 = spAudioSessionControl;
		if (spAudioSessionControl2 == NULL)
		{
			continue;
		}

		// Check if it is the firfox's audio session
		DWORD dwProcessId;
		EXIT_ON_ERROR(spAudioSessionControl2->GetProcessId(&dwProcessId));
		std::map<DWORD, BOOL>::iterator iter = map.find(dwProcessId);
		if (iter != map.end() && iter->second == TRUE)
		{
			CComQIPtr<ISimpleAudioVolume> spSimpleAudioVolume = spAudioSessionControl2;
			spSimpleAudioVolume->SetMute(bMute, &AudioVolumnCtx);
		}
	}

Exit:
	SAFE_RELEASE(m_spAudioEndpoint);
}
