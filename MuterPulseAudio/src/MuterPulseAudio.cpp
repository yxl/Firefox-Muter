/*
 ============================================================================
 Name        : MuterPulseAudio.c
 Author      : Yuan Xulei
 Version     :
 Copyright   : 
 Description : MuterPulseAudio in C, Ansi-style
 ============================================================================
 */

#include "../config.h"
#include "MuterPulseAudio.h"
#include "AudioMixerControl.h"

#include <stdio.h>

AudioMixerControl g_ctrl;

extern "C"
BOOL Initialize(void)
{
  bool ret = g_ctrl.Open();
  if (ret)
  {
    g_ctrl.EnableMute(false);
    printf("[%s] Initialized!\n", PACKAGE_NAME);
    return true;
  }
  return false;
}

extern "C"
void Dispose(void)
{
  g_ctrl.EnableMute(false);
  g_ctrl.Close();
  printf("[%s] Disposed!\n", PACKAGE_NAME);
}

extern "C"
void EnableMute(BOOL bEnabled)
{
  g_ctrl.EnableMute(bEnabled == TRUE);
}

extern "C"
int IsMuteEnabled()
{
  return g_ctrl.IsMute() ? TRUE: FALSE;
}
