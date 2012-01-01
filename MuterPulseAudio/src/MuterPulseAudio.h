/*
 * MuterPulseAudio.h
 *
 *  Created on: 2012-1-1
 *      Author: Yuan Xulei
 */

#ifndef MUTERPULSEAUDIO_H_
#define MUTERPULSEAUDIO_H_

#define BOOL int
#define TRUE 1
#define FALSE 0

BOOL Initialize(void);

void Dispose(void);

void EnableMute(BOOL bEnabled);

BOOL IsMuteEnabled();


#endif /* MUTERPULSEAUDIO_H_ */
