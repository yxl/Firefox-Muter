#include "ThTypes.h"

HANDLE MMFHandle = 0;
TGlobalDLLData *GlobalData = NULL;

void ThTypes_Init()
{
	if (!MMFHandle)
		MMFHandle = CreateFileMapping(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, sizeof(TGlobalDLLData), "muter@yxl.name/sharedmem");
	if (!GlobalData)
		GlobalData = static_cast<TGlobalDLLData *>(MapViewOfFile(MMFHandle, FILE_MAP_ALL_ACCESS, 0, 0, 0));
	if (GlobalData) 
	{
		GlobalData->bMute = FALSE;
    GlobalData->lLastSoundPlayingTimeInSeconds = -1;
	}
}

void ThTypes_End()
{
	if (GlobalData) 
	{
		GlobalData->bMute = FALSE;
    GlobalData->lLastSoundPlayingTimeInSeconds = -1;
		UnmapViewOfFile(GlobalData);
		GlobalData = NULL;
	}
	if (MMFHandle) 
	{
		CloseHandle(MMFHandle);
		MMFHandle = 0;
	}
}
