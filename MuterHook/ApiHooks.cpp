#include <windows.h>

#include "ThTypes.h"
#include "ApiHooks.h"
#include "HookImportFunction.h"
#include "HookMgr.h"

HookMgr g_hookMgr = HookMgr();

//
// Function pointers for basic hooks
//
typedef FARPROC (WINAPI *GetProcAddress_t)(HMODULE hmod, PCSTR pszProcName);
GetProcAddress_t GetProcAddress_next = NULL;

typedef HMODULE (WINAPI *LoadLibraryA_t)(PCSTR pszModulePath);
LoadLibraryA_t LoadLibraryA_next = NULL;

typedef HMODULE (WINAPI *LoadLibraryW_t)(PCWSTR pszModulePath);
LoadLibraryW_t LoadLibraryW_next = NULL;

typedef HMODULE (WINAPI *LoadLibraryExA_t)(PCSTR pszModulePath, HANDLE hFile, DWORD dwFlags);
LoadLibraryExA_t LoadLibraryExA_next = NULL;

typedef HMODULE (WINAPI *LoadLibraryExW_t)(PCWSTR pszModulePath, HANDLE hFile, DWORD dwFlags);
LoadLibraryExW_t LoadLibraryExW_next = NULL;

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

// Function pointers for muter hooks
typedef MMRESULT (WINAPI *waveOutWrite_t)(HWAVEOUT hwo, LPWAVEHDR pwh, UINT cbwh);
waveOutWrite_t waveOutWrite_next = NULL;

MMRESULT WINAPI waveOutWrite_hook(HWAVEOUT hwo, LPWAVEHDR pwh, UINT cbwh) {
  if (GlobalData && GlobalData->bMute) {
    memset(pwh->lpData, 0 , pwh->dwBufferLength);
  }

  return waveOutWrite_next(hwo, pwh, cbwh);
}

typedef BOOL (WINAPI *midiStreamOut_t)(HMIDISTRM hms, LPMIDIHDR pmh, UINT cbmh);
midiStreamOut_t midiStreamOut_next = NULL; 

MMRESULT WINAPI midiStreamOut_hook(HMIDISTRM hms, LPMIDIHDR pmh, UINT cbmh) {
  if (GlobalData && GlobalData->bMute) {
    memset(pmh->lpData, 0 , pmh->dwBufferLength);
  }

  return midiStreamOut_next(hms, pmh, cbmh);
}

void InstallBasicHooks()
{

}

void UnInstallBasicHooks()
{

}

void InstallMuterHooks()
{
  g_hookMgr.InstallHook("winmm.dll", "waveOutWrite", (PROC)waveOutWrite_hook, (PROC*)&waveOutWrite_next);
  g_hookMgr.InstallHook("winmm.dll", "midiStreamOut", (PROC)midiStreamOut_hook, (PROC*)&midiStreamOut_next);
}

void UnInstallMuterHooks()
{
  g_hookMgr.ClearAllHooks();
}
