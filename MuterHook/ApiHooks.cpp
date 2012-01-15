#include <windows.h>
#include <tlhelp32.h>

#include "external\detours.h"
#pragma comment(lib, "external\\lib\\detours.lib")

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
  PVOID*  ppOriginalFunction;
  PVOID   pHookFunction;
  BOOL    bSucceeded;
};

#define DEFINE_FUNCTION_INFO(module, func) {module, #func, (PVOID *)&func##_original, (PVOID)func##_hook, FALSE}

FunctionInfo s_Functions[] = 
{
  DEFINE_FUNCTION_INFO("Kernel32.dll", CreateProcessA),
  DEFINE_FUNCTION_INFO("Kernel32.dll", CreateProcessW),

  DEFINE_FUNCTION_INFO("winmm.dll", waveOutWrite),
  DEFINE_FUNCTION_INFO("winmm.dll", midiStreamOut),
  DEFINE_FUNCTION_INFO("dsound.dll", DirectSoundCreate),
  DEFINE_FUNCTION_INFO("dsound.dll", DirectSoundCreate8),

  DEFINE_FUNCTION_INFO("ole32.dll", CoCreateInstance),
};

const size_t s_FunctionsCount = sizeof(s_Functions)/sizeof(FunctionInfo);

BOOL InjectIntoProcess(HANDLE hProcess) 
{

  TRACE("[MuterHook] InjectIntoProcess\n");
  LPCSTR rlpDlls[2];
  DWORD nDlls = 0;
  rlpDlls[nDlls++] = g_szThisModulePath;
  return DetourUpdateProcessWithDll(hProcess, rlpDlls, nDlls);
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

  return TRUE;
}

void InstallMuterHooks()
{
  DetourRestoreAfterWith();
  DetourTransactionBegin();
  DetourUpdateThread(GetCurrentThread());

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

    *(info.ppOriginalFunction) = GetProcAddress(hModule, info.szFunctionName);
    if (*(info.ppOriginalFunction) == NULL)
    {
      TRACE("[MuterHook] Cannot GetProcAddress of %s", info.szFunctionName);
      continue;
    }

    if (NO_ERROR != DetourAttach(info.ppOriginalFunction, info.pHookFunction))
    {
      TRACE("[MuterHook] DetourAttach failed! Module: %s  Function: %s", info.szFunctionModule, info.szFunctionName);
      continue;
    }
    info.bSucceeded = TRUE;
  }

  DetourTransactionCommit();
}

void UnInstallMuterHooks()
{
  DetourTransactionBegin();
  DetourUpdateThread(GetCurrentThread());

  for(int i = 0; i < s_FunctionsCount; ++i)
  {
    FunctionInfo& info = s_Functions[i];
    if (*info.ppOriginalFunction == NULL)
    {
      DetourDetach(info.ppOriginalFunction, info.pHookFunction);
    }
  }

  DetourTransactionCommit();
}
