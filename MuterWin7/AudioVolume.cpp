#include "stdafx.h"
#include "AudioVolume.h"
#include "MuterWin7.h"

#define SAFE_RELEASE(sp) \
	if ((sp) != NULL) \
{ (sp).Release();}

AudioVolume::AudioVolume(void)
	: m_bRegisteredForEndpointNotifications(FALSE)
	, m_bRegisteredForAudioSessionNotifications(FALSE)
	, m_cRef(1)
{
	m_mapSpAudioSessionControl2.InitHashTable(257);
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
	TRACE("AudioVolume::Initialize Enters\n");
	HRESULT hr;

	// create enumerator
	hr = m_spEnumerator.CoCreateInstance(__uuidof(MMDeviceEnumerator));
	if (SUCCEEDED(hr))
	{
		hr = m_spEnumerator->RegisterEndpointNotificationCallback(this);
		if (SUCCEEDED(hr))
		{
			m_bRegisteredForEndpointNotifications = TRUE;
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
	TRACE("AudioVolume::Dispose Enters\n");

	DetachFromEndpoint();

	m_csEndpoint.Enter();
	if (m_bRegisteredForEndpointNotifications)
	{
		m_spEnumerator->UnregisterEndpointNotificationCallback(this);
		m_bRegisteredForEndpointNotifications = FALSE;
	}
	m_csEndpoint.Leave();

	SAFE_RELEASE(m_spEnumerator);
}


// ----------------------------------------------------------------------
//  Start monitoring the current default device
//
// ----------------------------------------------------------------------
HRESULT AudioVolume::AttachToDefaultEndpoint()
{
	TRACE("AudioVolume::AttachToDefaultEndpoint Enters\n");
	m_csEndpoint.Enter();

	// get the default music & movies playback device
	HRESULT hr = m_spEnumerator->GetDefaultAudioEndpoint(eRender, eMultimedia, &m_spAudioEndpoint);
	if (SUCCEEDED(hr))
	{
		// Get the session manager for this device.
		hr = m_spAudioEndpoint->Activate(__uuidof(IAudioSessionManager2), CLSCTX_INPROC_SERVER, NULL, reinterpret_cast<void **>(&m_spAudioSessionManager2));
		if (SUCCEEDED(hr))
		{
			hr = m_spAudioSessionManager2->RegisterSessionNotification(this);
			m_bRegisteredForAudioSessionNotifications = SUCCEEDED(hr);
			InitAudioSessionControlList();
			UpdateAudioSessionControlMuteStatus();
		}
	}

	m_csEndpoint.Leave();

	TRACE("AudioVolume::AttachToDefaultEndpoint Leaves\n");
	return hr;
}

// ----------------------------------------------------------------------
//  Stop monitoring the device and release all associated references
//
// ----------------------------------------------------------------------
void AudioVolume::DetachFromEndpoint()
{
	TRACE("AudioVolume::DetachFromEndpoint Enters\n");

	m_csEndpoint.Enter();

	DisposeAudioSessionControlList();
	if (m_spAudioSessionManager2 != NULL)
	{
		// be sure to unregister...
		if (m_bRegisteredForAudioSessionNotifications)
		{
			m_spAudioSessionManager2->UnregisterSessionNotification(this);
			m_bRegisteredForAudioSessionNotifications = FALSE;
		}
		SAFE_RELEASE(m_spAudioSessionManager2);
	}

	SAFE_RELEASE(m_spAudioEndpoint);

	m_csEndpoint.Leave();

	TRACE("AudioVolume::DetachFromEndpoint Leaves\n");
}

void AudioVolume::UpdateAudioSessionControlMuteStatus()
{
	POSITION pos = m_mapSpAudioSessionControl2.GetStartPosition();
	while (pos != NULL)
	{
		CComQIPtr<IAudioSessionControl> spAudioSessionControl = m_mapSpAudioSessionControl2.GetNextValue(pos);
		CComQIPtr<ISimpleAudioVolume> spSimpleAudioVolume = spAudioSessionControl;
		if (spSimpleAudioVolume != NULL)
		{
			spSimpleAudioVolume->SetMute(g_bEnableMute, &AudioVolumnCtx);
			spSimpleAudioVolume->SetMasterVolume(static_cast<float>(g_iVolume / 100.0f), &AudioVolumnCtx);
		}
	}
}

void AudioVolume::InitAudioSessionControlList()
{
	HRESULT hr = S_OK;

	if (m_spAudioSessionManager2 == NULL)
		return;

	CComQIPtr<IAudioSessionEnumerator> spAudioSessionEnumerator;
	try
	{
		// Get audio session enumerator
		hr = m_spAudioSessionManager2->GetSessionEnumerator(&spAudioSessionEnumerator);
		if (FAILED(hr))
		{
			throw "Cannot get audio session enumerator!";
		}

		// Get audio session number
		int nSessions = 0;
		hr = spAudioSessionEnumerator->GetCount(&nSessions);
		if (FAILED(hr))
		{
			throw "Cannot get audio session number!";
		}

		// Map indicating whether a process belongs to firefox
		std::map<DWORD, DWORD> map;
		if (!BuildProcesseTree(map))
		{
			throw "AudioVolume::GetSubProcesseMap failed!";
		}

		// Enumerate audio sessions
		for (int i=0; i<nSessions; i++)
		{
			CComQIPtr<IAudioSessionControl> spAudioSessionControl;
			try
			{
				// Get AudioSessionControl
				HRESULT hr = spAudioSessionEnumerator->GetSession(i, &spAudioSessionControl);
				if (FAILED(hr))
				{
					throw "Cannot get AudioSessionControl!";
				}

				AddSession(map, spAudioSessionControl);
			}
			catch (LPCSTR szError)
			{
				TRACE("[MuterWin7] AudioVolume::InitAudioSessionControlList: %s\n", szError);
			}
		}
	} 
	catch (LPCSTR szError)
	{
		TRACE("[MuterWin7] AudioVolume::InitAudioSessionControlList: %s\n", szError);
	}
	SAFE_RELEASE(spAudioSessionEnumerator);
}

void AudioVolume::AddSession(std::map<DWORD, DWORD> &map, CComQIPtr<IAudioSessionControl> spAudioSessionControl)
{
	HRESULT hr = S_OK;
	CComQIPtr<IAudioSessionControl2> spAudioSessionControl2;
	try
	{
		// Get AudioSessionControl2
		spAudioSessionControl2 = spAudioSessionControl;
		if (spAudioSessionControl2 == NULL)
		{
			throw "Cannot get AudioSessionControl2!";
		}

		// Check if it is the firfox's audio session
		DWORD dwProcessId;
		hr = spAudioSessionControl2->GetProcessId(&dwProcessId);
		if (FAILED(hr))
		{
			throw "spAudioSessionControl2->GetProcessId failed!";
		}
		LPWSTR pswSessionName = NULL;
		hr = spAudioSessionControl->GetDisplayName(&pswSessionName);
		if (FAILED(hr))
		{
			throw "spAudioSessionControl->GetDisplayName failed!";
		}				
		if (IsDescendantProcess(map, dwProcessId) || IsQzoneMusicProcess(dwProcessId))
		{
			LPWSTR pswInstanceId = NULL;
			if (SUCCEEDED(spAudioSessionControl2->GetSessionInstanceIdentifier(&pswInstanceId)))
			{
				m_mapSpAudioSessionControl2[CStringW(pswInstanceId)] = spAudioSessionControl;
			}
			CoTaskMemFree(pswInstanceId);
		}

		CoTaskMemFree(pswSessionName);	
	}
	catch (LPCSTR szError)
	{
		TRACE("[MuterWin7] AudioVolume::AddSession: %s\n", szError);
	}
}

void AudioVolume::DisposeAudioSessionControlList()
{
	POSITION pos = m_mapSpAudioSessionControl2.GetStartPosition();
	while (pos != NULL)
	{
		CComQIPtr<IAudioSessionControl> spAudioSessionControl = m_mapSpAudioSessionControl2.GetNextValue(pos);
		SAFE_RELEASE(spAudioSessionControl);
	}
	m_mapSpAudioSessionControl2.RemoveAll();
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
	TRACE("AudioVolume::OnDefaultDeviceChanged Enters\n");
	m_csEndpoint.Enter();
	if (flow == eRender && role == eMultimedia)
	{
		if (g_uThread)
		{
			::PostThreadMessage(g_uThread, MSG_USER_DEFAULT_DEVICE_CHANGE, 0, 0);
		}
	}
	m_csEndpoint.Leave();
	TRACE("AudioVolume::OnDefaultDeviceChanged Leaves\n");
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
	TRACE("AudioVolume::OnSessionCreated Enters\n");
	m_csEndpoint.Enter();

	CComQIPtr<IAudioSessionControl> spIAudioSessionControl = NewSession;
	if (spIAudioSessionControl != NULL)
	{
		try
		{
			// Map indicating whether a process belongs to firefox
			std::map<DWORD, DWORD> map;
			if (!BuildProcesseTree(map))
			{
				throw "AudioVolume::GetSubProcesseMap failed!";
			}

			AddSession(map, spIAudioSessionControl);

			if (g_uThread)
			{
				::PostThreadMessage(g_uThread, MSG_USER_UPDATE_MUTE_STATUS, 0, 0);
			}
		}
		catch (LPCSTR szError)
		{
			TRACE("[MuterWin7] AudioVolume::OnSessionCreated: %s\n", szError);
		}
	}

	m_csEndpoint.Leave();
	TRACE("AudioVolume::OnSessionCreated Leaves\n");
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

// Get a map that its keys contains all process IDs and the value for each key is the subprocess ID.
BOOL AudioVolume::BuildProcesseTree(std::map<DWORD, DWORD> &map)
{
	HANDLE hSnapShot = ::CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	if (hSnapShot == INVALID_HANDLE_VALUE)
	{
		return FALSE;
	}

	PROCESSENTRY32 procentry  = { sizeof(PROCESSENTRY32) };
	BOOL bContinue = Process32First(hSnapShot, &procentry);
	while(bContinue)
	{
		DWORD dwProcessId = procentry.th32ProcessID;
		if (dwProcessId > 0)
		{
			map.insert(std::make_pair(dwProcessId, procentry.th32ParentProcessID));
			_ASSERT(procentry.th32ProcessID != procentry.th32ParentProcessID);
		}
		bContinue = Process32Next( hSnapShot, &procentry );
	}

	CloseHandle(hSnapShot);
	return TRUE;
}

BOOL AudioVolume::IsQzoneMusicProcess(DWORD processId) 
{
	HANDLE hrocess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ , FALSE, processId);
	WCHAR fileName[MAX_PATH];
	DWORD ret = GetModuleFileNameExW(hrocess, NULL, fileName, MAX_PATH);
	BOOL bFind = (ret > 0 && wcsstr(fileName, L"QzoneMusic.exe") != NULL);
	CloseHandle(hrocess);
	return bFind;
}

BOOL AudioVolume::IsDescendantProcess(std::map<DWORD, DWORD> &map, DWORD processId, int depth /*= 8*/) 
{
	if (depth < 1)
	{
		return FALSE;
	}
	if (g_dwThisModuleProcessId == processId)
	{
		return TRUE;
	}
	if (processId == 0)
	{
		return FALSE;
	}
	DWORD parent = 0;
	auto iter = map.find(processId);
	if (iter != map.end() &&
		iter->second != 0 &&
		iter->second != processId)
	{
		if (iter->second == g_dwThisModuleProcessId)
		{
			return TRUE;
		}
		DWORD parentID = iter->second;
		// Set its parent to 0 to avoid reiterate the node 
		iter->second = 0;
		if (IsDescendantProcess(map, parentID, depth - 1))
		{
			iter->second = g_dwThisModuleProcessId;
			return TRUE;
		}
		return FALSE;
	} 
	else 
	{
		if (iter != map.end())
		{
			iter->second = 0;
		}
		return FALSE;
	}
}


// Update mute status of all the audio sessions
void AudioVolume::UpdateMuteStatus()
{
	TRACE("[MuterWin7] AudioVolume::UpdateMuteStatus Enters\n");

	m_csEndpoint.Enter();
	UpdateAudioSessionControlMuteStatus();
	m_csEndpoint.Leave();

	TRACE("[MuterWin7] AudioVolume::UpdateMuteStatus Leaves\n");
}