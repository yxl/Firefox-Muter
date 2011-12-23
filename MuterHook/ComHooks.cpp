#include "ComHooks.h"
#include "ApiHooks.h"
#include "SDKTrace.h"

HRESULT WINAPI  CoGetClassObject(REFCLSID rclsid, DWORD dwClsContext, LPVOID pvReserved,
							REFIID riid, LPVOID FAR* ppv);