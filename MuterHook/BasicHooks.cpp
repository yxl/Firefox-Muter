#include "BasicHooks.h"
#include "ApiHooks.h"
#include "DllEntry.h"
#include "SDKTrace.h"

LPCWSTR IGNORE_PLUGIN_LIST_WIDE[] = 
{
	L"npaliedit.dll",
	L"npqqedit.dll",
	L"npqqcert.dll",
};

LPCWSTR HOOK_PROCESS_LIST_WIDE[] = 
{
	L"plugin-container.exe"
};

BOOL IsIgnored(LPCWSTR lpCommandLine)
{
	if (lpCommandLine == NULL)
	{
		return TRUE;
	}
	int n = sizeof(HOOK_PROCESS_LIST_WIDE)/sizeof(LPCWSTR);
	int i = 0;
	for (i = 0; i<n; i++)
	{
		if (wcsstr(lpCommandLine, HOOK_PROCESS_LIST_WIDE[i]) != NULL)
		{
			break;
		}
	}
	if (i == n)
	{
		return TRUE;
	}
	n = sizeof(IGNORE_PLUGIN_LIST_WIDE)/sizeof(LPCWSTR);
	for (i = 0; i<n; i++)
	{
		if (wcsstr(lpCommandLine, IGNORE_PLUGIN_LIST_WIDE[i]) != NULL)
		{
			return TRUE;
		}
	}
	return FALSE;
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
	TRACE("[MuterHook] CreateProcessW_hook lpCommandLine=%s\n", lpCommandLine);

	BOOL ret = ::CreateProcessW_original(lpApplicationName, lpCommandLine, 
		lpProcessAttributes, lpThreadAttributes, bInheritHandles, 
		dwCreationFlags, lpEnvironment, lpCurrentDirectory, lpStartupInfo,
		lpProcessInformation);
	// Check if we need hook.
	if (ret && !IsIgnored(lpCommandLine))
	{
		InjectIntoProcess(lpProcessInformation->hProcess);
	}
	return ret;
}