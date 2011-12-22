#include "ThTypes.h"
#include "BasicHooks.h"
#include "ApiHooks.h"
#include "DllEntry.h"
#include "HookMgr.h"
#include "SDKTrace.h"

FARPROC WINAPI GetProcAddress_hook(HMODULE hmod, PCSTR pszProcName) 
{
	// Get the true address of the function
	FARPROC pfn = ::GetProcAddress(hmod, pszProcName);
	// Is it one of the functions that we want hooked?
	HookItem *pItem = g_hookMgr.FindHook(hmod, pfn);
	if (pItem && pItem->pOrigFunc)
		return pItem->pHookFunc;
	else
		return pfn;
}


HMODULE WINAPI LoadLibraryA_hook(PCSTR pszModulePath) 
{
	TRACE("[MuterHook] LoadLibraryA_hook(%s)\n", pszModulePath);
	HMODULE hmod = ::LoadLibraryA(pszModulePath);
	if (strstr(pszModulePath, DLL_NAME) != NULL)
		return hmod;
	if (hmod != GetThisModule())
		InstallHooksForNewModule(hmod);
	return(hmod);
}

HMODULE WINAPI LoadLibraryW_hook(PCWSTR pszModulePath) 
{
	TRACE_WIDE(L"[MuterHook] LoadLibraryW_hook(%s)\n", pszModulePath);
	HMODULE hmod = ::LoadLibraryW(pszModulePath);
	if (wcsstr(pszModulePath, DLL_NAME_WIDE) != NULL)
		return hmod;
	if (hmod != GetThisModule())
		InstallHooksForNewModule(hmod);
	return(hmod);
}

HMODULE WINAPI LoadLibraryExA_hook(PCSTR pszModulePath, 
								   HANDLE hFile, DWORD dwFlags) 
{
	TRACE("[MuterHook] LoadLibraryExA_hook(%s)\n", pszModulePath);
	HMODULE hmod = ::LoadLibraryExA(pszModulePath, hFile, dwFlags);
	if (hmod != GetThisModule() && (dwFlags & LOAD_LIBRARY_AS_DATAFILE) == 0)
		InstallHooksForNewModule(hmod);
	return(hmod);
}

HMODULE WINAPI LoadLibraryExW_hook(PCWSTR pszModulePath, 
								   HANDLE hFile, DWORD dwFlags) 
{
	TRACE_WIDE(L"[MuterHook] LoadLibraryExW_hook(%s)\n", pszModulePath);
	HMODULE hmod = ::LoadLibraryExW(pszModulePath, hFile, dwFlags);
	if (hmod != GetThisModule() && (dwFlags & LOAD_LIBRARY_AS_DATAFILE) == 0)
		InstallHooksForNewModule(hmod);
	return(hmod);
}

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
			strstr(lpCommandLine, "npaliedit.dll") == NULL &&
			strstr(lpCommandLine, "npqqedit.dll") == NULL &&
			strstr(lpCommandLine, "npqqcert.dll") == NULL)
		{
			InjectIntoProcess(lpProcessInformation->hProcess);
		}
	}
	return ret;
}

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
			wcsstr(lpCommandLine, L"npaliedit.dll") == NULL &&
			wcsstr(lpCommandLine, L"npqqedit.dll") == NULL && 
			wcsstr(lpCommandLine, L"npqqcert.dll") == NULL)
		{
			InjectIntoProcess(lpProcessInformation->hProcess);
		}
	}
	return ret;
}