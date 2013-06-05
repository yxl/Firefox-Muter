#include "stdafx.h"
#include "ComHooks.h"
#include "ApiHooks.h"
#include "QQMusicHook.h"
#include "DllEntry.h"

HRESULT (WINAPI *CoCreateInstance_original)(REFCLSID rclsid, LPUNKNOWN pUnkOuter, DWORD dwClsContext, REFIID riid, LPVOID* ppv) = NULL;


HRESULT WINAPI CoCreateInstance_hook(REFCLSID rclsid, LPUNKNOWN pUnkOuter, DWORD dwClsContext, REFIID riid, LPVOID* ppv)
{
  TRACE("[MuterHook] CoCreateInstance_hook\n");
  HRESULT hret =  CoCreateInstance_original(rclsid, pUnkOuter, dwClsContext, riid, ppv);
  InstallMuterHooks();
  PostThreadMessage(g_uThread, WM_USER_HOOK_PROCESSES, 0, 0);
  return hret;
}