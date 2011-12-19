#include <windows.h>

#include "ThTypes.h"
#include "ApiHooks.h"
#include "HookImportFunction.h"
#include "HookMgr.h"
#include "MuterHook.h"
#include "mydsound.h"

#define DLL_NAME "MuterHook.dll"
#define DLL_NAME_WIDE L"MuterHook.dll"
HookMgr g_hookMgr = HookMgr();

HMODULE ModuleFromAddress(PVOID pv) 
{
  MEMORY_BASIC_INFORMATION mbi;
  return((VirtualQuery(pv, &mbi, sizeof(mbi)) != 0) 
    ? (HMODULE) mbi.AllocationBase : NULL);
}

//
// Basic function hooks
//
typedef FARPROC (WINAPI *GetProcAddress_t)(HMODULE hmod, PCSTR pszProcName);
GetProcAddress_t GetProcAddress_next = NULL;

FARPROC WINAPI GetProcAddress_hook(HMODULE hmod, PCSTR pszProcName) {

  // Get the true address of the function
  FARPROC pfn = GetProcAddress_next(hmod, pszProcName);
  // Is it one of the functions that we want hooked?
  HookItem *pItem = g_hookMgr.FindHookByOrignalFunc(pfn);
  if (pItem)
    return pItem->pHookFunc;
  else
    return pfn;
}

typedef HMODULE (WINAPI *LoadLibraryA_t)(PCSTR pszModulePath);
LoadLibraryA_t LoadLibraryA_next = NULL;

HMODULE WINAPI LoadLibraryA_hook(PCSTR pszModulePath) {
  HMODULE hmod = ::LoadLibraryA_next(pszModulePath);
  if (strstr(pszModulePath, DLL_NAME) != NULL)
    return hmod;
  if (hmod != ModuleFromAddress(ModuleFromAddress))
    g_hookMgr.UpdateAllHooksForNewModule(hmod, 0);
  return(hmod);
}

typedef HMODULE (WINAPI *LoadLibraryW_t)(PCWSTR pszModulePath);
LoadLibraryW_t LoadLibraryW_next = NULL;

HMODULE WINAPI LoadLibraryW_hook(PCWSTR pszModulePath) {
  HMODULE hmod = ::LoadLibraryW_next(pszModulePath);
  if (wcsstr(pszModulePath, DLL_NAME_WIDE) != NULL)
    return hmod;
  if (hmod != ModuleFromAddress(ModuleFromAddress))
    g_hookMgr.UpdateAllHooksForNewModule(hmod, 0);
  return(hmod);
}


typedef HMODULE (WINAPI *LoadLibraryExA_t)(PCSTR pszModulePath, HANDLE hFile, DWORD dwFlags);
LoadLibraryExA_t LoadLibraryExA_next = NULL;

HMODULE WINAPI LoadLibraryExA_hook(PCSTR pszModulePath, 
                                   HANDLE hFile, DWORD dwFlags) 
{
  HMODULE hmod = ::LoadLibraryExA_next(pszModulePath, hFile, dwFlags);
  if (hmod != ModuleFromAddress(ModuleFromAddress))
    g_hookMgr.UpdateAllHooksForNewModule(hmod, dwFlags);
  return(hmod);
}

typedef HMODULE (WINAPI *LoadLibraryExW_t)(PCWSTR pszModulePath, HANDLE hFile, DWORD dwFlags);
LoadLibraryExW_t LoadLibraryExW_next = NULL;

HMODULE WINAPI LoadLibraryExW_hook(PCWSTR pszModulePath, 
                                   HANDLE hFile, DWORD dwFlags) 
{
  HMODULE hmod = ::LoadLibraryExW_next(pszModulePath, hFile, dwFlags);
  if (hmod != ModuleFromAddress(ModuleFromAddress))
    g_hookMgr.UpdateAllHooksForNewModule(hmod, dwFlags);
  return(hmod);
}


BOOL WINAPI InjectIntoProcess(HANDLE hprocess) {
  TCHAR dllpath[MAX_PATH];
  LPVOID memory_pointer = VirtualAllocEx(hprocess, NULL, 
    sizeof(dllpath), MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);

  if (!memory_pointer) 
  {
    return FALSE;
  }

  if (!g_hInstance)
  {
    g_hInstance = ModuleFromAddress(InjectIntoProcess);
  }
  GetModuleFileName(g_hInstance, dllpath, MAX_PATH);
  if (!WriteProcessMemory(hprocess, memory_pointer, dllpath, 
    sizeof(dllpath), NULL))
  {
    return FALSE;
  }

  HANDLE remote_thread = CreateRemoteThread(hprocess, NULL, 0,
    (LPTHREAD_START_ROUTINE)LoadLibrary, memory_pointer, 0, NULL);
  if (!remote_thread) 
  {
    return FALSE;
  }

  CloseHandle(remote_thread);

  return TRUE;
}


typedef BOOL (WINAPI *CreateProcessA_t)(LPCSTR lpApplicationName, 
                                        LPSTR lpCommandLine, 
                                        LPSECURITY_ATTRIBUTES lpProcessAttributes, 
                                        LPSECURITY_ATTRIBUTES lpThreadAttributes, 
                                        BOOL bInheritHandles, 
                                        DWORD dwCreationFlags,
                                        LPVOID lpEnvironment, 
                                        LPCSTR lpCurrentDirectory,
                                        LPSTARTUPINFOA lpStartupInfo,
                                        LPPROCESS_INFORMATION lpProcessInformation);
CreateProcessA_t CreateProcessA_next = NULL;

BOOL WINAPI CreateProcessA_hook(LPCSTR lpApplicationName, 
                                LPSTR lpCommandLine, 
                                LPSECURITY_ATTRIBUTES lpProcessAttributes, 
                                LPSECURITY_ATTRIBUTES lpThreadAttributes, 
                                BOOL bInheritHandles, 
                                DWORD dwCreationFlags,
                                LPVOID lpEnvironment, 
                                LPCSTR lpCurrentDirectory,
                                LPSTARTUPINFOA lpStartupInfo,
                                LPPROCESS_INFORMATION lpProcessInformation) 
{
  BOOL ret = ::CreateProcessA_next(lpApplicationName, lpCommandLine, 
    lpProcessAttributes, lpThreadAttributes, bInheritHandles, 
    dwCreationFlags, lpEnvironment, lpCurrentDirectory, lpStartupInfo,
    lpProcessInformation);
  if (ret) 
  {
    if (lpCommandLine != NULL && strstr(lpApplicationName, "plugin-container.exe") != NULL &&
      strstr(lpCommandLine, "npaliedit.dll") == NULL)
    {
      InjectIntoProcess(lpProcessInformation->hProcess);
    }
  }
  return ret;
}


typedef BOOL (WINAPI *CreateProcessW_t)(LPCWSTR lpApplicationName, 
                                        LPWSTR lpCommandLine, 
                                        LPSECURITY_ATTRIBUTES lpProcessAttributes, 
                                        LPSECURITY_ATTRIBUTES lpThreadAttributes, 
                                        BOOL bInheritHandles, 
                                        DWORD dwCreationFlags,
                                        LPVOID lpEnvironment, 
                                        LPCWSTR lpCurrentDirectory,
                                        LPSTARTUPINFOW lpStartupInfo,
                                        LPPROCESS_INFORMATION lpProcessInformation);
CreateProcessW_t CreateProcessW_next = NULL;

BOOL WINAPI CreateProcessW_hook(LPCWSTR lpApplicationName, 
                                LPWSTR lpCommandLine, 
                                LPSECURITY_ATTRIBUTES lpProcessAttributes, 
                                LPSECURITY_ATTRIBUTES lpThreadAttributes, 
                                BOOL bInheritHandles, 
                                DWORD dwCreationFlags,
                                LPVOID lpEnvironment, 
                                LPCWSTR lpCurrentDirectory,
                                LPSTARTUPINFOW lpStartupInfo,
                                LPPROCESS_INFORMATION lpProcessInformation) 
{
  BOOL ret = ::CreateProcessW_next(lpApplicationName, lpCommandLine, 
    lpProcessAttributes, lpThreadAttributes, bInheritHandles, 
    dwCreationFlags, lpEnvironment, lpCurrentDirectory, lpStartupInfo,
    lpProcessInformation);
  if (ret) 
  {
    if (lpCommandLine != NULL && wcsstr(lpCommandLine, L"plugin-container.exe") != NULL &&
      wcsstr(lpCommandLine, L"npaliedit.dll") == NULL)
    {
      InjectIntoProcess(lpProcessInformation->hProcess);
    }
  }
  return ret;
}

// Sound function hooks
typedef MMRESULT (WINAPI *waveOutWrite_t)(HWAVEOUT hwo, LPWAVEHDR pwh, UINT cbwh);
waveOutWrite_t waveOutWrite_next = NULL;

MMRESULT WINAPI waveOutWrite_hook(HWAVEOUT hwo, LPWAVEHDR pwh, UINT cbwh) 
{
  if (GlobalData && GlobalData->bMute) {
    memset(pwh->lpData, 0 , pwh->dwBufferLength);
  }

  return waveOutWrite_next(hwo, pwh, cbwh);
}

typedef BOOL (WINAPI *midiStreamOut_t)(HMIDISTRM hms, LPMIDIHDR pmh, UINT cbmh);
midiStreamOut_t midiStreamOut_next = NULL; 

MMRESULT WINAPI midiStreamOut_hook(HMIDISTRM hms, LPMIDIHDR pmh, UINT cbmh) 
{
  if (GlobalData && GlobalData->bMute) {
    memset(pmh->lpData, 0 , pmh->dwBufferLength);
  }

  return midiStreamOut_next(hms, pmh, cbmh);
}

typedef int (WINAPI* DirectSoundCreate_t)(LPCGUID pcGuidDevice, 
                                          LPDIRECTSOUND *ppDS, 
                                          LPUNKNOWN pUnkOuter);
DirectSoundCreate_t DirectSoundCreate_next = NULL;

HRESULT WINAPI DirectSoundCreate_hook(LPCGUID pcGuidDevice, 
                                      LPDIRECTSOUND *ppDS, 
                                      LPUNKNOWN pUnkOuter) 
{

  HRESULT hr = DirectSoundCreate_next(pcGuidDevice, ppDS, pUnkOuter);

  if (SUCCEEDED(hr)) 
  {
    MyDirectSound* p = new MyDirectSound;
    p->direct_sound_ = *ppDS;
    *ppDS = p;

  }
  return hr;
}

typedef int (WINAPI* DirectSoundCreate8_t)(LPCGUID pcGuidDevice, 
                                           LPDIRECTSOUND8 *ppDS, 
                                           LPUNKNOWN pUnkOuter);
DirectSoundCreate8_t DirectSoundCreate8_next = NULL;

HRESULT WINAPI DirectSoundCreate8_hook(LPCGUID pcGuidDevice, 
                                       LPDIRECTSOUND8 *ppDS, 
                                       LPUNKNOWN pUnkOuter) 
{

  HRESULT hr = DirectSoundCreate8_next(pcGuidDevice, ppDS, pUnkOuter);

  if (SUCCEEDED(hr)) 
  {
    MyDirectSound8* p = new MyDirectSound8;
    p->direct_sound_ = *ppDS;
    *ppDS = p;
  }
  return hr;
}


void InstallMuterHooks()
{
  // Basic function hooks

  /*
  if (GetProcAddress_next == NULL)
  {
    g_hookMgr.InstallHook("Kernel32.dll", "GetProcAddress", (PROC)GetProcAddress_hook, (PROC*)&GetProcAddress_next);
  }*/
  if (LoadLibraryA_next == NULL)
  {
    g_hookMgr.InstallHook("Kernel32.dll", "LoadLibraryA", (PROC)LoadLibraryA_hook, (PROC*)&LoadLibraryA_next);
  }
  if (LoadLibraryW_next == NULL)
  {
    g_hookMgr.InstallHook("Kernel32.dll", "LoadLibraryW", (PROC)LoadLibraryW_hook, (PROC*)&LoadLibraryW_next);
  }
  if (LoadLibraryExA_next == NULL)
  {
    g_hookMgr.InstallHook("Kernel32.dll", "LoadLibraryExA", (PROC)LoadLibraryExA_hook, (PROC*)&LoadLibraryExA_next);
  }
  if (LoadLibraryExW_next == NULL)
  {
    g_hookMgr.InstallHook("Kernel32.dll", "LoadLibraryExW", (PROC)LoadLibraryExW_hook, (PROC*)&LoadLibraryExW_next);
  }
  if (CreateProcessA_next == NULL)
  {
    g_hookMgr.InstallHook("Kernel32.dll", "CreateProcessA", (PROC)CreateProcessA_hook, (PROC*)&CreateProcessA_next);
  }
  if (CreateProcessW_next == NULL)
  {
    g_hookMgr.InstallHook("Kernel32.dll", "CreateProcessW", (PROC)CreateProcessW_hook, (PROC*)&CreateProcessW_next);
  }

  // Sound function hooks
  if (waveOutWrite_next == NULL)
  {
    g_hookMgr.InstallHook("winmm.dll", "waveOutWrite", (PROC)waveOutWrite_hook, (PROC*)&waveOutWrite_next);
  }
  if (midiStreamOut_next == NULL)
  {
    g_hookMgr.InstallHook("winmm.dll", "midiStreamOut", (PROC)midiStreamOut_hook, (PROC*)&midiStreamOut_next);
  }
  if (DirectSoundCreate_next == NULL)
  {
    g_hookMgr.InstallHook("dsound.dll", "DirectSoundCreate", (PROC)DirectSoundCreate_hook, (PROC*)&DirectSoundCreate_next);
  }
  if (DirectSoundCreate8_next == NULL)
  {
    g_hookMgr.InstallHook("dsound.dll", "DirectSoundCreate8", (PROC)DirectSoundCreate8_hook, (PROC*)&DirectSoundCreate8_next);
  }
}

void UnInstallMuterHooks()
{
  g_hookMgr.ClearAllHooks();
}
