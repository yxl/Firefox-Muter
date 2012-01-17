#include "BasicHooks.h"
#include "ApiHooks.h"
#include "DllEntry.h"
#include "SDKTrace.h"
#include "external\detours.h"

LPCSTR IGNORE_PLUGIN_LIST[] = 
{
  "npaliedit.dll",
  "npqqedit.dll",
  "npqqcert.dll",
};

LPCWSTR IGNORE_PLUGIN_LIST_WIDE[] = 
{
  L"npaliedit.dll",
  L"npqqedit.dll",
  L"npqqcert.dll",
};

BOOL IsIgnored(LPCSTR lpCommandLine)
{
  if (lpCommandLine == NULL || strstr(lpCommandLine, "plugin-container.exe") == NULL)
  {
    return TRUE;
  }
  int n = sizeof(IGNORE_PLUGIN_LIST)/sizeof(LPCSTR);
  for (int i = 0; i<n; i++)
  {
    if (strstr(lpCommandLine, IGNORE_PLUGIN_LIST[i]) != NULL)
    {
      return TRUE;
    }
  }
  return FALSE;
}

BOOL IsIgnored(LPCWSTR lpCommandLine)
{
  if (lpCommandLine == NULL || wcsstr(lpCommandLine, L"plugin-container.exe") == NULL)
  {
    return TRUE;
  }
  int n = sizeof(IGNORE_PLUGIN_LIST_WIDE)/sizeof(LPCWSTR);
  for (int i = 0; i<n; i++)
  {
    if (wcsstr(lpCommandLine, IGNORE_PLUGIN_LIST_WIDE[i]) != NULL)
    {
      return TRUE;
    }
  }
  return FALSE;
}

LPCSTR FORCE_INJECTION_PLUGIN_LIST[] = 
{
  "npqtplugin.dll"
};

LPCWSTR FORCE_INJECTION_PLUGIN_LIST_WIDE[] = 
{
  L"npqtplugin.dll"
};

BOOL NeedForceInjection(LPCSTR lpCommandLine)
{
  int n = sizeof(FORCE_INJECTION_PLUGIN_LIST)/sizeof(LPCSTR);
  for (int i = 0; i<n; i++)
  {
    if (strstr(lpCommandLine, FORCE_INJECTION_PLUGIN_LIST[i]) != NULL)
    {
      return TRUE;
    }
  }
  return FALSE;
}

BOOL NeedForceInjection(LPCWSTR lpCommandLine)
{
  int n = sizeof(FORCE_INJECTION_PLUGIN_LIST_WIDE)/sizeof(LPCWSTR);
  for (int i = 0; i<n; i++)
  {
    if (wcsstr(lpCommandLine, FORCE_INJECTION_PLUGIN_LIST_WIDE[i]) != NULL)
    {
      return TRUE;
    }
  }
  return FALSE;
}

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
  TRACE("[MuterHook] CreateProcessA_hook lpCommandLine=%s\n", lpCommandLine);

  // Check if we need hook.
  if (!IsInThisModuleProcess() || IsIgnored(lpCommandLine))
  {
    return CreateProcessA_original(lpApplicationName, lpCommandLine, 
      lpProcessAttributes, lpThreadAttributes, bInheritHandles, 
      dwCreationFlags, lpEnvironment, lpCurrentDirectory, lpStartupInfo,
      lpProcessInformation);
  }

  // Check if we need hook by force
  if (NeedForceInjection(lpCommandLine))
  {
    BOOL ret = ::CreateProcessA_original(lpApplicationName, lpCommandLine, 
      lpProcessAttributes, lpThreadAttributes, bInheritHandles, 
      dwCreationFlags, lpEnvironment, lpCurrentDirectory, lpStartupInfo,
      lpProcessInformation);
    if (ret)
    {
      InjectIntoProcessByForce(lpProcessInformation->hProcess);
    }
    return ret;
  }
  else 
  {
    return DetourCreateProcessWithDllA(lpApplicationName, lpCommandLine, lpProcessAttributes, 
    lpThreadAttributes, bInheritHandles,dwCreationFlags, lpEnvironment, lpCurrentDirectory,
    lpStartupInfo, lpProcessInformation, g_szThisModulePath, CreateProcessA_original);
  }
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
  TRACE_WIDE(L"[MuterHook] CreateProcessA_hook lpCommandLine=%s\n", lpCommandLine);

  // Check if we need hook.
  if (!IsInThisModuleProcess() || IsIgnored(lpCommandLine))
  {
    return CreateProcessW_original(lpApplicationName, lpCommandLine, 
      lpProcessAttributes, lpThreadAttributes, bInheritHandles, 
      dwCreationFlags, lpEnvironment, lpCurrentDirectory, lpStartupInfo,
      lpProcessInformation);
  }

  // Check if we need hook by force
  if (NeedForceInjection(lpCommandLine))
  {
    BOOL ret = ::CreateProcessW_original(lpApplicationName, lpCommandLine, 
      lpProcessAttributes, lpThreadAttributes, bInheritHandles, 
      dwCreationFlags, lpEnvironment, lpCurrentDirectory, lpStartupInfo,
      lpProcessInformation);
    if (ret)
    {
      InjectIntoProcessByForce(lpProcessInformation->hProcess);
    }
    return ret;
  }
  else 
  {
    return DetourCreateProcessWithDllW(lpApplicationName, lpCommandLine, lpProcessAttributes, 
    lpThreadAttributes, bInheritHandles,dwCreationFlags, lpEnvironment, lpCurrentDirectory,
    lpStartupInfo, lpProcessInformation, g_szThisModulePath, CreateProcessW_original);
  }
}