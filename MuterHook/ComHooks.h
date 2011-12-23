#pragma once

#include <Windows.h>

HRESULT WINAPI  CoGetClassObject_hook(REFCLSID rclsid, DWORD dwClsContext, LPVOID pvReserved,
                                      REFIID riid, LPVOID FAR* ppv);

HRESULT WINAPI CoCreateInstance_hook(REFCLSID rclsid, LPUNKNOWN pUnkOuter, DWORD dwClsContext, REFIID riid, LPVOID* ppv);