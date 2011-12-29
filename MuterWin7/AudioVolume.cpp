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
  DetachFromEndpoint();

  if (m_bRegisteredForEndpointNotifications)
  {
    m_spEnumerator->UnregisterEndpointNotificationCallback(this);
    m_bRegisteredForEndpointNotifications = FALSE;
  }

  SAFE_RELEASE(m_spEnumerator);
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
      InitializeAudioSessionControlList();
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

  DisposeAudioSessionControlList();

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

void AudioVolume::InitializeAudioSessionControlList()
{
  HRESULT hr = S_OK;

  CComQIPtr<IAudioSessionEnumerator> spAudioSessionEnumerator;
  try
  {
    // Map indicating whether a process belongs to firefox
    std::map<DWORD, BOOL> map;
    if (!GetSubProcesseMap(g_dwThisModuleProcessId, map))
    {
      throw "AudioVolume::GetSubProcesseMap failed!";
    }
    std::map<DWORD, BOOL>::iterator iter = map.find(g_dwThisModuleProcessId);
    if (iter != map.end())
    {
      iter->second = TRUE;
    }

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

    // Enumerate audio sessions
    for (int i=0; i<nSessions; i++)
    {
      CComQIPtr<IAudioSessionControl> spAudioSessionControl;
      CComQIPtr<IAudioSessionControl2> spAudioSessionControl2;
      try
      {
        // Get AudioSessionControl
        CComQIPtr<IAudioSessionControl> spAudioSessionControl;
        HRESULT hr = spAudioSessionEnumerator->GetSession(i, &spAudioSessionControl);
        if (FAILED(hr))
        {
          throw "Cannot get AudioSessionControl!";
        }

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
        std::map<DWORD, BOOL>::iterator iter = map.find(dwProcessId);
        if (iter != map.end() && iter->second == TRUE)
        {
          m_arrSpAudioSessionControl2.Add(spAudioSessionControl2);
        }
      }
      catch (LPTSTR szError)
      {
        TRACE("[MuterWin7] AudioVolume::InitializeAudioSessionControlList: %s\n", szError);
      }
      SAFE_RELEASE(spAudioSessionControl2);
      SAFE_RELEASE(spAudioSessionControl);
    }
  } 
  catch (LPTSTR szError)
  {
    TRACE("[MuterWin7] AudioVolume::InitializeAudioSessionControlList: %s\n", szError);
  }
  SAFE_RELEASE(spAudioSessionEnumerator);
}

void AudioVolume::DisposeAudioSessionControlList()
{
  int n = m_arrSpAudioSessionControl2.GetCount();
  for (int i=0; i<n; i++)
  {
    SAFE_RELEASE(m_arrSpAudioSessionControl2[i]);
  }
  m_arrSpAudioSessionControl2.RemoveAll();
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
//  Implementation of IAudioSessionNotification::OnSessionCreated
//
//  Notifies the registered processes that the audio session has been created.
//
// ----------------------------------------------------------------------
HRESULT AudioVolume::OnSessionCreated(IAudioSessionControl *NewSession)
{
  TRACE("AudioVolume::OnSessionCreated\n");

  m_csEndpoint.Enter();

  CComQIPtr<IAudioSessionControl2> spIAudioSessionControl2 = NewSession;

  try
  {
    // Map indicating whether a process belongs to firefox
    std::map<DWORD, BOOL> map;
    if (!GetSubProcesseMap(g_dwThisModuleProcessId, map))
    {
      throw "AudioVolume::GetSubProcesseMap failed!";
    }
    std::map<DWORD, BOOL>::iterator iter = map.find(g_dwThisModuleProcessId);
    if (iter != map.end())
    {
      iter->second = TRUE;
    }

    // Check if it is the firfox's audio session
    DWORD dwProcessId;
    HRESULT hr = spIAudioSessionControl2->GetProcessId(&dwProcessId);
    if (FAILED(hr)) 
    {
      throw "spAudioSessionControl2->GetProcessId failed!";
    }
    iter = map.find(dwProcessId);
    if (iter != map.end() && iter->second == TRUE)
    {
      m_arrSpAudioSessionControl2.Add(spIAudioSessionControl2);
    }

    UpdateMuteStatus();
  }
  catch (LPCSTR szError)
  {
    TRACE("[MuterWin7] AudioVolume::OnSessionCreated: %s\n", szError);
  }

  SAFE_RELEASE(spIAudioSessionControl2);

  m_csEndpoint.Leave();
  
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

  BOOL bMute = ::IsMuteEnabled();

  int n = m_arrSpAudioSessionControl2.GetCount();

  // Enumerate audio sessions
  for (int i=0; i<n; i++)
  {
    CComQIPtr<ISimpleAudioVolume> spSimpleAudioVolume = m_arrSpAudioSessionControl2[i];
    spSimpleAudioVolume->SetMute(bMute, &AudioVolumnCtx);
    SAFE_RELEASE(spSimpleAudioVolume);
  }
}
