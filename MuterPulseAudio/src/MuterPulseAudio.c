/*
 ============================================================================
 Name        : MuterPulseAudio.c
 Author      : Yuan Xulei
 Version     :
 Copyright   : 
 Description : MuterPulseAudio in C, Ansi-style
 ============================================================================
 */

#include "MuterPulseAudio.h"

BOOL g_bMute = FALSE;

BOOL Initialize(void)
{
  return TRUE;
}

void Dispose(void)
{

}

void EnableMute(BOOL bEnabled)
{
  g_bMute = bEnabled;
}

int IsMuteEnabled()
{
  return g_bMute;
}
