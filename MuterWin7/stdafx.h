// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

#include "targetver.h"

#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers
// Windows Header Files:
#include <windows.h>

#include <windows.h>
#include <atlbase.h>
#include <atlsync.h>
#include <atlcoll.h>
#include <atlstr.h>

#include <mmdeviceapi.h>
#include <audiopolicy.h>

#include <shlwapi.h>
#pragma comment(lib, "Shlwapi.lib")

// Enumerate processes
#include <tlhelp32.h>

// GetModuleFileNameExW
#include <psapi.h>
#pragma comment(lib, "Psapi.lib")

#include <map>

#include "SDKTrace.h"
