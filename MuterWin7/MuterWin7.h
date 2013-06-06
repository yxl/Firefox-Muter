#ifdef __cplusplus
extern "C"
{
#endif				/* __cplusplus */

	// The following ifdef block is the standard way of creating macros which make exporting 
	// from a DLL simpler. All files within this DLL are compiled with the MUTERWIN7_EXPORTS
	// symbol defined on the command line. This symbol should not be defined on any project
	// that uses this DLL. This way any other project whose source files include this file see 
	// MUTERWIN7_API functions as being imported from a DLL, whereas this DLL sees symbols
	// defined with this macro as being exported.
#ifdef MUTERWIN7_EXPORTS
#define MUTERWIN7_API __declspec(dllexport)
#else
#define MUTERWIN7_API __declspec(dllimport)
#endif

	// Custom thread message ID
#define MSG_USER_DEFAULT_DEVICE_CHANGE (WM_USER + 200)
#define MSG_USER_UPDATE_MUTE_STATUS (WM_USER + 201)

	MUTERWIN7_API BOOL Initialize(void);

	MUTERWIN7_API void Dispose(void);

	MUTERWIN7_API void EnableMute(BOOL bEnabled);

	MUTERWIN7_API BOOL IsMuteEnabled();

	/**
	 * Max volume - 100
	 * Min volume - 0
	 */
	MUTERWIN7_API int GetVolume();

	MUTERWIN7_API void SetVolume(int iVolume);

	extern const int MIN_VOLUME /*= 0*/;
	extern const int MAX_VOLUME /*= 100*/;

	extern unsigned g_uThread;
	
	extern DWORD g_dwThisModuleProcessId;

	extern int g_iVolume;

	extern BOOL g_bEnableMute;
#ifdef __cplusplus
}
#endif				/* __cplusplus */
