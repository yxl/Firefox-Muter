#include "BasicHooks.h"
#include "ApiHooks.h"
#include "DllEntry.h"
#include "SDKTrace.h"
#include "external\detours.h"

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
	BOOL ret= ::DetourCreateProcessWithDllA(lpApplicationName, lpCommandLine, lpProcessAttributes, 
		lpThreadAttributes, bInheritHandles,dwCreationFlags, lpEnvironment, lpCurrentDirectory,
		lpStartupInfo, lpProcessInformation, g_szThisModulePath, CreateProcessA_original);
	if (ret == FALSE)
	{
		TRACE("[MuterHook] CreateProcessA_hook failed!\n");
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
	BOOL ret = ::DetourCreateProcessWithDllW(lpApplicationName, lpCommandLine, 
		lpProcessAttributes, lpThreadAttributes, bInheritHandles, 
		dwCreationFlags, lpEnvironment, lpCurrentDirectory, lpStartupInfo,
		lpProcessInformation, g_szThisModulePath, CreateProcessW_original);
	if (ret == FALSE)
	{
		TRACE("[MuterHook] CreateProcessW_hook failed!\n");
	}
	return ret;
}