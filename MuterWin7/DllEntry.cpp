// DllEntry.cpp : Defines the entry point for the DLL application.
#include "stdafx.h"
#include "MuterWin7.h"

DWORD g_dwThisModuleProcessId = 0;

BOOL APIENTRY DllMain( HMODULE hModule,
  DWORD  ul_reason_for_call,
  LPVOID lpReserved
  )
{
  switch (ul_reason_for_call)
  {
  case DLL_PROCESS_ATTACH:
    {
      g_dwThisModuleProcessId = GetCurrentProcessId();
      HRESULT hr = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);
      if (FAILED(hr))
      {
        return FALSE;
      }
    }
    break;
  case DLL_PROCESS_DETACH:
    CoUninitialize();
    break;
  case DLL_THREAD_ATTACH:
    break;
  case DLL_THREAD_DETACH:
    break;
  }
  return TRUE;
}

