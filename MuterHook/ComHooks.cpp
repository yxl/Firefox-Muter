#include "ComHooks.h"
#include "ApiHooks.h"
#include "SDKTrace.h"

HRESULT WINAPI  CoGetClassObject_hook(REFCLSID rclsid, DWORD dwClsContext, LPVOID pvReserved,
                                      REFIID riid, LPVOID FAR* ppv)
{
  TRACE("[MuterHook] CoGetClassObject_hook\n");
  ::InstallMuterHooks();
  HRESULT hret =  ::CoGetClassObject(rclsid, dwClsContext, pvReserved, riid, ppv);
  ::InstallMuterHooks();
  return hret;
}

HRESULT WINAPI CoCreateInstance_hook(REFCLSID rclsid, LPUNKNOWN pUnkOuter, DWORD dwClsContext, REFIID riid, LPVOID* ppv)
{
  TRACE("[MuterHook] CoCreateInstance_hook\n");
  HRESULT hret =  ::CoCreateInstance(rclsid, pUnkOuter, dwClsContext, riid, ppv);
  ::InstallMuterHooks();
  return hret;
}