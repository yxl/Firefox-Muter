#pragma once

#include <windows.h>

BOOL HookAPI(LPCSTR szImportModule, LPCSTR szFunc, PROC paHookFuncs, PROC* paOrigFuncs);