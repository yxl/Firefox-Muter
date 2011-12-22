#ifndef _ThTypes_H_
#define _ThTypes_H_

#include <windows.h>

#ifdef __cplusplus
extern "C"
{
#endif				/* __cplusplus */

	typedef struct TGlobalDLLData 
	{
		BOOL bMute;
    LONG lLastSoundPlayingTimeInSeconds;
	} TGlobalDLLData;

	extern TGlobalDLLData *GlobalData;


	void ThTypes_Init();
	void ThTypes_End();

#ifdef __cplusplus
}
#endif				/* __cplusplus */

#endif
