#include <windows.h>
#include <tlhelp32.h>

#include "external\MinHook.h"

#if defined(_DEBUG)
	#define MIN_HOOK_LIB_SURFFIX "d.lib"
#else
	#define MIN_HOOK_LIB_SURFFIX ".lib"
#endif
#ifndef _M_X64
#pragma comment(lib, "external\\lib\\libMinHook32" MIN_HOOK_LIB_SURFFIX)
#else
#pragma comment(lib, "external\\lib\\libMinHook64" MIN_HOOK_LIB_SURFFIX)
#endif

#include "ApiHooks.h"
#include "DllEntry.h"
#include "BasicHooks.h"
#include "SoundHooks.h"
#include "ComHooks.h"
#include "SDKTrace.h"

struct FunctionInfo
{
	LPCSTR  szFunctionModule;
	LPCSTR  szFunctionName;
	PVOID   pTargetFunction;
	PVOID*  ppOriginalFunction;
	PVOID   pHookFunction;
	BOOL    bSucceeded;
};

#define DEFINE_FUNCTION_INFO(module, func) {module, #func, NULL, reinterpret_cast<PVOID *>(&func##_original), reinterpret_cast<PVOID>(func##_hook), FALSE}


FunctionInfo s_Functions[] = 
{
	DEFINE_FUNCTION_INFO("Kernel32.dll", CreateProcessW),

	DEFINE_FUNCTION_INFO("ole32.dll", CoCreateInstance),

	DEFINE_FUNCTION_INFO("winmm.dll", waveOutWrite),
	DEFINE_FUNCTION_INFO("winmm.dll", midiStreamOut),
	DEFINE_FUNCTION_INFO("dsound.dll", DirectSoundCreate),
	DEFINE_FUNCTION_INFO("dsound.dll", DirectSoundCreate8),
};

const size_t s_FunctionsCount = sizeof(s_Functions)/sizeof(FunctionInfo);

BOOL InjectIntoProcess(HANDLE hProcess) 
{
	BOOL bOK = FALSE;

	LPVOID pPath = NULL;
	HANDLE hThread = NULL;
	try
	{
		pPath = VirtualAllocEx(hProcess, NULL, 
			sizeof(g_szThisModulePath), MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
		if (!pPath) 
		{
			throw "VirtualAllocEx failed!";
		}

		if (!WriteProcessMemory(hProcess, pPath, g_szThisModulePath, 
			sizeof(g_szThisModulePath), NULL))
		{
			throw "WriteProcessMemory failed!";
		}

		hThread = CreateRemoteThread(hProcess, NULL, 0,
			(LPTHREAD_START_ROUTINE)LoadLibraryA, pPath, 0, NULL);
		if (!hThread)
		{
			throw "CreateRemoteThread failed!";
		}

		WaitForSingleObject(hThread, INFINITE);

		bOK = TRUE;

	}
	catch (LPSTR error)
	{
		TRACE("[MuterHook] InjectIntoProcessByForce failed! %s\n", error);
	}

	if (hThread)
	{
		CloseHandle(hThread);
	}

	if (pPath)
	{
		VirtualFreeEx(hProcess, pPath, 0, MEM_RELEASE);
	}

	return bOK;
}

void InjectIntoSubProcesses()
{
	DWORD dwCurrentProcessId = GetCurrentProcessId();
	HANDLE hSnapShot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	if (hSnapShot == INVALID_HANDLE_VALUE)
	{
		return;
	}

	PROCESSENTRY32 procentry  = { sizeof(PROCESSENTRY32) };
	BOOL bContinue = Process32First(hSnapShot, &procentry);
	while( bContinue )
	{
		if(dwCurrentProcessId == procentry.th32ParentProcessID)
		{
			HANDLE hProcess = OpenProcess(
				PROCESS_CREATE_THREAD | PROCESS_QUERY_INFORMATION | 
				PROCESS_VM_OPERATION | PROCESS_VM_WRITE | PROCESS_VM_READ, 
				FALSE, procentry.th32ProcessID);
			if (hProcess)
			{	
				InjectIntoProcess(hProcess);
				CloseHandle(hProcess);
			}
			else
			{
				TRACE("[MuterHook] InjectIntoSubProcesses failed!, ProcessId=%ld\n", procentry.th32ProcessID);
			}
		}
		bContinue = Process32Next( hSnapShot, &procentry );
	}

	CloseHandle(hSnapShot);
}

BOOL IsInThisModuleProcess()
{
	DWORD dwCurrentProcessId = GetCurrentProcessId();

	if (dwCurrentProcessId == g_dwThisModuleProcessId) 
		return TRUE;

	return FALSE;
}

void InstallMuterHooks()
{
	if (MH_Initialize() != MH_OK)
	{
		return;
	}
	for(int i = 0; i < s_FunctionsCount; ++i)
	{
		FunctionInfo& info = s_Functions[i];

		if (info.bSucceeded) 
		{
			continue;
		}

		HMODULE hModule = ::LoadLibraryA(info.szFunctionModule);
		if (!hModule)
		{
			DWORD dwErrorCode = ::GetLastError();
			TRACE("[MuterHook] Cannot LoadLibraryA(%s)! GetLastError: %d", info.szFunctionModule, dwErrorCode);
			continue;
		}

		info.pTargetFunction = ::GetProcAddress(hModule, info.szFunctionName);
		if (info.pTargetFunction == NULL)
		{
			TRACE("[MuterHook] Cannot GetProcAddress of %s", info.szFunctionName);
			continue;
		}

		if (::MH_CreateHook(info.pTargetFunction, info.pHookFunction, info.ppOriginalFunction) != MH_OK)
		{
			TRACE("[MuterHook] MH_CreateHook failed! Module: %s  Function: %s", info.szFunctionModule, info.szFunctionName);
			continue;
		}

		// Enable the hook
		if (::MH_EnableHook(info.pTargetFunction) != MH_OK)
		{
			TRACE("[fireie] MH_EnableHook failed! Module: %s  Function: %s", info.szFunctionModule, info.szFunctionName);
			continue;
		}
		info.bSucceeded = TRUE;
	}
}

void UnInstallMuterHooks()
{
	::MH_DisableAllHooks();
	for(int i = 0; i < s_FunctionsCount; ++i)
	{
		FunctionInfo& info = s_Functions[i];
		if (*info.ppOriginalFunction != NULL)
		{
			::MH_RemoveHook(info.pTargetFunction);
		}
	}
	MH_Uninitialize();
}
