#include "stdafx.h"
#include "ApiHooks.h"
#include "DllEntry.h"
#include "BasicHooks.h"
#include "SoundHooks.h"
#include "ComHooks.h"

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
	DEFINE_FUNCTION_INFO("winmm.dll", waveOutOpen),
	DEFINE_FUNCTION_INFO("winmm.dll", waveOutClose),
	DEFINE_FUNCTION_INFO("winmm.dll", waveOutGetVolume),
	DEFINE_FUNCTION_INFO("winmm.dll", waveOutSetVolume),
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

// Get a map that its keys contains all process IDs and the value for each key is the subprocess ID.
BOOL BuildProcesseTree(std::map<DWORD, DWORD> &map)
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

BOOL IsDescendantProcess(std::map<DWORD, DWORD> &map, DWORD processId, int depth = 8) 
{
	static DWORD s_dwThisModuleProcessId = GetCurrentProcessId();
	if (depth < 1)
	{
		return FALSE;
	}
	if (s_dwThisModuleProcessId == processId)
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
		if (iter->second == s_dwThisModuleProcessId)
		{
			return TRUE;
		}
		DWORD parentID = iter->second;
		// Set its parent to 0 to avoid reiterate the node 
		iter->second = 0;
		if (IsDescendantProcess(map, parentID, depth - 1))
		{
			iter->second = s_dwThisModuleProcessId;
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

void InjectIntoDescendantProcesses()
{
	// Map indicating whether a process belongs to firefox
	std::map<DWORD, DWORD> map;
	if (!BuildProcesseTree(map))
	{
		return;
	}

	HANDLE hSnapShot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	if (hSnapShot == INVALID_HANDLE_VALUE)
	{
		return;
	}

	PROCESSENTRY32 procentry  = { sizeof(PROCESSENTRY32) };
	BOOL bContinue = Process32First(hSnapShot, &procentry);
	while( bContinue )
	{
		if(IsDescendantProcess(map, procentry.th32ParentProcessID))
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
				TRACE("[MuterHook] InjectIntoDescendantProcesses failed!, ProcessId=%ld\n", procentry.th32ProcessID);
			}
		}
		bContinue = Process32Next( hSnapShot, &procentry );
	}

	CloseHandle(hSnapShot);
}

void InstallMuterHooks()
{
	if (MH_Initialize() != MH_OK)
	{
		TRACE("InstallMuterHooks already initialized.\n");
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
