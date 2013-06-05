// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

#include "targetver.h"

// Windows Header Files:
#include <windows.h>

#include <dsound.h>

// Enumerate processes
#include <tlhelp32.h>

// GetModuleFileNameExW
#include <psapi.h>
#pragma comment(lib, "Psapi.lib")

#include <Mmsystem.h>
#pragma comment(lib, "Winmm.lib")

// CCriticalSection
#include <atlbase.h>
#include <atlsync.h>

#include <tchar.h>
#include <stdio.h>
#include <stdarg.h>
// _beginthreadex & _endthreadex
#include <process.h>

#include <map>
//#include "external\dsound.h"

#include "external\MinHook.h"

#if defined(_DEBUG)
	#define MIN_HOOK_LIB_SURFFIX "d.lib"
#else
	#define MIN_HOOK_LIB_SURFFIX ".lib"
#endif
#ifndef _M_X64
#pragma comment(lib, "external\\lib\\libMinHook32" MIN_HOOK_LIB_SURFFIX)
#else
#pragma comment(lib, "external\\lib\\libMinHook64" MIN_HOOK_LIB_SURFFIX)
#endif

#include "SDKTrace.h"
