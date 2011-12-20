#include <windows.h>
#include <time.h>

#include "ThTypes.h"
#include "ApiHooks.h"
#include "HookImportFunction.h"
#include "HookMgr.h"
#include "MuterHook.h"
#include "mydsound.h"

HookMgr g_hookMgr = HookMgr();

void InstallHooksForNewModule(HMODULE hModule);

HMODULE GetModule() 
{
	MEMORY_BASIC_INFORMATION mbi;
	return((VirtualQuery(InstallHooksForNewModule, &mbi, sizeof(mbi)) != 0) 
		? (HMODULE) mbi.AllocationBase : NULL);
}

//
// Basic function hooks
//
typedef FARPROC (WINAPI *GetProcAddress_t)(HMODULE hmod, PCSTR pszProcName);

FARPROC WINAPI GetProcAddress_hook(HMODULE hmod, PCSTR pszProcName) {

	// Get the true address of the function
	FARPROC pfn = ::GetProcAddress(hmod, pszProcName);
	// Is it one of the functions that we want hooked?
	HookItem *pItem = g_hookMgr.FindHook(hmod, pfn);
	if (pItem && pItem->pOrigFunc)
		return pItem->pHookFunc;
	else
		return pfn;
}

typedef HMODULE (WINAPI *LoadLibraryA_t)(PCSTR pszModulePath);

HMODULE WINAPI LoadLibraryA_hook(PCSTR pszModulePath) 
{
	HMODULE hmod = ::LoadLibraryA(pszModulePath);
	if (strstr(pszModulePath, DLL_NAME) != NULL)
		return hmod;
	if (hmod != GetModule())
		InstallHooksForNewModule(hmod);
	return(hmod);
}

typedef HMODULE (WINAPI *LoadLibraryW_t)(PCWSTR pszModulePath);

HMODULE WINAPI LoadLibraryW_hook(PCWSTR pszModulePath) 
{
	HMODULE hmod = ::LoadLibraryW(pszModulePath);
	if (wcsstr(pszModulePath, DLL_NAME_WIDE) != NULL)
		return hmod;
	if (hmod != GetModule())
		InstallHooksForNewModule(hmod);
	return(hmod);
}


typedef HMODULE (WINAPI *LoadLibraryExA_t)(PCSTR pszModulePath, HANDLE hFile, DWORD dwFlags);

HMODULE WINAPI LoadLibraryExA_hook(PCSTR pszModulePath, 
								   HANDLE hFile, DWORD dwFlags) 
{
	HMODULE hmod = ::LoadLibraryExA(pszModulePath, hFile, dwFlags);
	if (hmod != GetModule() && (dwFlags & LOAD_LIBRARY_AS_DATAFILE) == 0)
		InstallHooksForNewModule(hmod);
	return(hmod);
}

typedef HMODULE (WINAPI *LoadLibraryExW_t)(PCWSTR pszModulePath, HANDLE hFile, DWORD dwFlags);

HMODULE WINAPI LoadLibraryExW_hook(PCWSTR pszModulePath, 
								   HANDLE hFile, DWORD dwFlags) 
{
	HMODULE hmod = ::LoadLibraryExW(pszModulePath, hFile, dwFlags);
	if (hmod != GetModule() && (dwFlags & LOAD_LIBRARY_AS_DATAFILE) == 0)
		InstallHooksForNewModule(hmod);
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
		g_hInstance = GetModule();
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
	BOOL ret = ::CreateProcessA(lpApplicationName, lpCommandLine, 
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
	BOOL ret = ::CreateProcessW(lpApplicationName, lpCommandLine, 
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

MMRESULT WINAPI waveOutWrite_hook(HWAVEOUT hwo, LPWAVEHDR pwh, UINT cbwh) 
{
  if (GlobalData)
  {
    GlobalData->lLastSoundPlayingTimeInSeconds = (LONG)time(NULL);
  }
	if (GlobalData && GlobalData->bMute) 
  { 
		memset(pwh->lpData, 0 , pwh->dwBufferLength);
	}

	waveOutWrite_t waveOutWrite_next = (waveOutWrite_t)g_hookMgr.GetOriginalFunc((PROC)waveOutWrite_hook);
	return waveOutWrite_next(hwo, pwh, cbwh);
}

typedef BOOL (WINAPI *midiStreamOut_t)(HMIDISTRM hms, LPMIDIHDR pmh, UINT cbmh);

MMRESULT WINAPI midiStreamOut_hook(HMIDISTRM hms, LPMIDIHDR pmh, UINT cbmh) 
{
  if (GlobalData)
  {
    GlobalData->lLastSoundPlayingTimeInSeconds = (LONG)time(NULL);
  }
	if (GlobalData && GlobalData->bMute) 
  {
		memset(pmh->lpData, 0 , pmh->dwBufferLength);
	}

	midiStreamOut_t midiStreamOut_next = (midiStreamOut_t)g_hookMgr.GetOriginalFunc((PROC)midiStreamOut_hook);
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
	DirectSoundCreate_t DirectSoundCreate_next = (DirectSoundCreate_t)g_hookMgr.GetOriginalFunc((PROC)DirectSoundCreate_hook);

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

HRESULT WINAPI DirectSoundCreate8_hook(LPCGUID pcGuidDevice, 
									   LPDIRECTSOUND8 *ppDS, 
									   LPUNKNOWN pUnkOuter) 
{
	DirectSoundCreate8_t DirectSoundCreate8_next = (DirectSoundCreate8_t)g_hookMgr.GetOriginalFunc((PROC)DirectSoundCreate8_hook);

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
	g_hookMgr.InstallHookForAllModules("Kernel32.dll", "GetProcAddress", (PROC)GetProcAddress_hook);
	g_hookMgr.InstallHookForAllModules("Kernel32.dll", "LoadLibraryA", (PROC)LoadLibraryA_hook);
	g_hookMgr.InstallHookForAllModules("Kernel32.dll", "LoadLibraryW", (PROC)LoadLibraryW_hook);
	g_hookMgr.InstallHookForAllModules("Kernel32.dll", "LoadLibraryExA", (PROC)LoadLibraryExA_hook);
	g_hookMgr.InstallHookForAllModules("Kernel32.dll", "LoadLibraryExW", (PROC)LoadLibraryExW_hook);
	g_hookMgr.InstallHookForAllModules("Kernel32.dll", "CreateProcessA", (PROC)CreateProcessA_hook);
	g_hookMgr.InstallHookForAllModules("Kernel32.dll", "CreateProcessW", (PROC)CreateProcessW_hook);

	// Sound function hooks
	g_hookMgr.InstallHookForAllModules("winmm.dll", "waveOutWrite", (PROC)waveOutWrite_hook);
	g_hookMgr.InstallHookForAllModules("winmm.dll", "midiStreamOut", (PROC)midiStreamOut_hook);
	g_hookMgr.InstallHookForAllModules("dsound.dll", "DirectSoundCreate", (PROC)DirectSoundCreate_hook);
	g_hookMgr.InstallHookForAllModules("dsound.dll", "DirectSoundCreate8", (PROC)DirectSoundCreate8_hook);
}

void UnInstallMuterHooks()
{
	g_hookMgr.ClearAllHooks();
}

void InstallHooksForNewModule(HMODULE hModule)
{
	// Basic function hooks
	g_hookMgr.InstallHookForOneModule(hModule, "Kernel32.dll", "GetProcAddress", (PROC)GetProcAddress_hook);
	g_hookMgr.InstallHookForOneModule(hModule, "Kernel32.dll", "LoadLibraryA", (PROC)LoadLibraryA_hook);
	g_hookMgr.InstallHookForOneModule(hModule, "Kernel32.dll", "LoadLibraryW", (PROC)LoadLibraryW_hook);
	g_hookMgr.InstallHookForOneModule(hModule, "Kernel32.dll", "LoadLibraryExA", (PROC)LoadLibraryExA_hook);
	g_hookMgr.InstallHookForOneModule(hModule, "Kernel32.dll", "LoadLibraryExW", (PROC)LoadLibraryExW_hook);
	g_hookMgr.InstallHookForOneModule(hModule, "Kernel32.dll", "CreateProcessA", (PROC)CreateProcessA_hook);
	g_hookMgr.InstallHookForOneModule(hModule, "Kernel32.dll", "CreateProcessW", (PROC)CreateProcessW_hook);

	// Sound function hooks
	g_hookMgr.InstallHookForOneModule(hModule, "winmm.dll", "waveOutWrite", (PROC)waveOutWrite_hook);
	g_hookMgr.InstallHookForOneModule(hModule, "winmm.dll", "midiStreamOut", (PROC)midiStreamOut_hook);
	g_hookMgr.InstallHookForOneModule(hModule, "dsound.dll", "DirectSoundCreate", (PROC)DirectSoundCreate_hook);
	g_hookMgr.InstallHookForOneModule(hModule, "dsound.dll", "DirectSoundCreate8", (PROC)DirectSoundCreate8_hook);
}