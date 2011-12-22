#pragma once
#include <windows.h>

#ifdef __cplusplus
extern "C"
{
#endif				/* __cplusplus */

// Should be capitilized!
#define DLL_NAME "MUTERHOOK.DLL"
#define DLL_NAME_WIDE L"MUTERHOOK.DLL"

#if BUILDING_DLL
# define DLLIMPORT __declspec (dllexport)
#else /* Not BUILDING_DLL */
# define DLLIMPORT __declspec (dllimport)
#endif /* Not BUILDING_DLL */

DLLIMPORT void EnableMute(BOOL bEnabled);

DLLIMPORT BOOL IsMuteEnabled();

DLLIMPORT LONG GetLastSoundPlayingTimeInSeconds();

extern HINSTANCE g_hInstance;

#ifdef __cplusplus
}
#endif				/* __cplusplus */