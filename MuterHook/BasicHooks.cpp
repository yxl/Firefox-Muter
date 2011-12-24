#include "BasicHooks.h"
#include "ApiHooks.h"
#include "DllEntry.h"
#include "SDKTrace.h"

BOOL (WINAPI *CreateProcessA_original)(LPCSTR lpApplicationName, 
											  LPSTR lpCommandLine, 
											  LPSECURITY_ATTRIBUTES lpProcessAttributes, 
											  LPSECURITY_ATTRIBUTES lpThreadAttributes, 
											  BOOL bInheritHandles, 
											  DWORD dwCreationFlags,
											  LPVOID lpEnvironment, 
											  LPCSTR lpCurrentDirectory,
											  LPSTARTUPINFOA lpStartupInfo,
											  LPPROCESS_INFORMATION lpProcessInformation) = NULL;

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
	TRACE("[MuterHook] CreateProcessA_hook\n");
	BOOL ret = ::CreateProcessA_original(lpApplicationName, lpCommandLine, 
		lpProcessAttributes, lpThreadAttributes, bInheritHandles, 
		dwCreationFlags, lpEnvironment, lpCurrentDirectory, lpStartupInfo,
		lpProcessInformation);
	if (ret) 
	{
		if (IsInThisModuleProcess() && lpCommandLine != NULL && strstr(lpApplicationName, "plugin-container.exe") != NULL &&
			strstr(lpCommandLine, "npaliedit.dll") == NULL &&
			strstr(lpCommandLine, "npqqedit.dll") == NULL &&
			strstr(lpCommandLine, "npqqcert.dll") == NULL)
		{
			InjectIntoProcess(lpProcessInformation->hProcess);
		}
	}
	return ret;
}

BOOL (WINAPI *CreateProcessW_original)(LPCWSTR lpApplicationName, 
											  LPWSTR lpCommandLine, 
											  LPSECURITY_ATTRIBUTES lpProcessAttributes, 
											  LPSECURITY_ATTRIBUTES lpThreadAttributes, 
											  BOOL bInheritHandles, 
											  DWORD dwCreationFlags,
											  LPVOID lpEnvironment, 
											  LPCWSTR lpCurrentDirectory,
											  LPSTARTUPINFOW lpStartupInfo,
											  LPPROCESS_INFORMATION lpProcessInformation) = NULL;

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
	TRACE("[MuterHook] CreateProcessW_hook\n");
	BOOL ret = ::CreateProcessW_original(lpApplicationName, lpCommandLine, 
		lpProcessAttributes, lpThreadAttributes, bInheritHandles, 
		dwCreationFlags, lpEnvironment, lpCurrentDirectory, lpStartupInfo,
		lpProcessInformation);
	if (ret) 
	{
		if (IsInThisModuleProcess() &&lpCommandLine != NULL && wcsstr(lpCommandLine, L"plugin-container.exe") != NULL &&
			wcsstr(lpCommandLine, L"npaliedit.dll") == NULL &&
			wcsstr(lpCommandLine, L"npqqedit.dll") == NULL && 
			wcsstr(lpCommandLine, L"npqqcert.dll") == NULL)
		{
			InjectIntoProcess(lpProcessInformation->hProcess);
		}
	}
	return ret;
}