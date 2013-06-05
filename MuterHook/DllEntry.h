#pragma once

#ifdef __cplusplus
extern "C"
{
#endif				/* __cplusplus */

#if BUILDING_DLL
# define DLLIMPORT __declspec (dllexport)
#else /* Not BUILDING_DLL */
# define DLLIMPORT __declspec (dllimport)
#endif /* Not BUILDING_DLL */

	DLLIMPORT BOOL Initialize(void);

	DLLIMPORT void Dispose(void);

	DLLIMPORT void EnableMute(BOOL bEnabled);

	DLLIMPORT BOOL IsMuteEnabled();

	// Custom thread message ID
	extern const UINT WM_USER_HOOK_PROCESSES;

	extern CHAR g_szThisModulePath[MAX_PATH];

	extern unsigned g_uThread;

#ifdef __cplusplus
}
#endif				/* __cplusplus */