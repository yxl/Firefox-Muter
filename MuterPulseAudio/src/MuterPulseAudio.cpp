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
#include "AudioMixerControl.h"


AudioMixerControl g_ctrl;

extern "C"
BOOL Initialize(void)
{
  bool ret = g_ctrl.Open();
  if (ret)
  {
    g_ctrl.EnableMute(false);
    return true;
  }
  return false;
}

extern "C"
void Dispose(void)
{
  g_ctrl.EnableMute(false);
  g_ctrl.Close();
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
