#include "ComHooks.h"
#include "ApiHooks.h"
#include "SDKTrace.h"

HRESULT (WINAPI *CoCreateInstance_original)(REFCLSID rclsid, LPUNKNOWN pUnkOuter, DWORD dwClsContext, REFIID riid, LPVOID* ppv) = NULL;


HRESULT WINAPI CoCreateInstance_hook(REFCLSID rclsid, LPUNKNOWN pUnkOuter, DWORD dwClsContext, REFIID riid, LPVOID* ppv)
{
  TRACE("[MuterHook] CoCreateInstance_hook\n");
  HRESULT hret =  CoCreateInstance_original(rclsid, pUnkOuter, dwClsContext, riid, ppv);
  InstallMuterHooks();
  InjectIntoSubProcesses();
  return hret;
}