#pragma once

#include <windows.h>

BOOL HookAPI(LPCSTR szImportModule, LPCSTR szFunc, PROC paHookFuncs, PROC* paOrigFuncs);

BOOL HookImportFunction(HMODULE hModule, LPCSTR szImportModule, LPCSTR szFunc, PROC pHookFunc, PROC* ppOrigFunc);