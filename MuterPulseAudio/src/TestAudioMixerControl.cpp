/*
 * TestAudioMixerControl.cpp
 *
 *  Created on: 2012-1-3
 *      Author: Yuan Xulei
 */
#include <stdio.h>
#include "AudioMixerControl.h"

int main()
{
  AudioMixerControl ctrl;
  ctrl.Open();

  printf("Open\n");

  // Input q to exit and m to switch mute status
  int c = 0;
  bool muter = false;
  while (c != 'q')
  {
    c = getchar();
    if (c == 'm' || c == 'M')
    {
      muter = !muter;
      ctrl.EnableMute(muter);
    }
  }

  ctrl.Close();

  printf("Close\n");

  return 0;
}

