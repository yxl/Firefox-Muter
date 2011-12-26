#include "AudioVolume.h"
#include "DllEntry.h"

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
		hr = m_spAudioEndpoint->Activate(__uuidof(IAudioSessionManager), CLSCTX_INPROC_SERVER, NULL, (void**)&m_spAudioSessionManager);
		if (SUCCEEDED(hr))
		{
			// Get simple audio volume 
			hr = m_spAudioSessionManager->GetSimpleAudioVolume(&GUID_NULL, TRUE, &m_spSimpleAudioVolume);

			// Get the audio session. 
			hr = m_spAudioSessionManager->GetAudioSessionControl( 
				&GUID_NULL,     // Get the default audio session. 
				TRUE ,          // The session is cross-process.
				&m_spAudioSessionControl);

			if (SUCCEEDED(hr))
			{
				// register for callbacks
				//hr = m_spAudioSessionControl->RegisterAudioSessionNotification(this);
				//m_bRegisteredForAudioSessionNotifications = SUCCEEDED(hr);
			}
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

	if (m_spAudioSessionControl != NULL)
	{
		// be sure to unregister...
		if (m_bRegisteredForAudioSessionNotifications)
		{
			m_spAudioSessionControl->UnregisterAudioSessionNotification(this);
			m_bRegisteredForAudioSessionNotifications = FALSE;
		}

		m_spAudioSessionControl.Release();
	}

	if (m_spSimpleAudioVolume != NULL)
	{
		m_spSimpleAudioVolume.Release();
	}

	if (m_spAudioSessionManager != NULL)
	{
		m_spAudioSessionManager.Release();
	}

	if (m_spAudioEndpoint != NULL)
	{
		m_spAudioEndpoint.Release();
	}

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
	BOOL bMute = ShouldMute();
	if (bMute != NewMute)
	{
		EnableMute(bMute);
	}
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

void AudioVolume::EnableMute( BOOL bEnabled )
{
	if (m_spSimpleAudioVolume != NULL)
	{
		m_spSimpleAudioVolume->SetMute(bEnabled, &AudioVolumnCtx);
	}
}
