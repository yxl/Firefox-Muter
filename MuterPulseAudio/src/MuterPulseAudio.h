/*
 * MuterPulseAudio.h
 *
 *  Created on: 2012-1-1
 *      Author: Yuan Xulei
 */

#ifndef MUTERPULSEAUDIO_H_
#define MUTERPULSEAUDIO_H_

#ifdef __cplusplus
extern "C"
{
#endif                          /* __cplusplus */

#define BOOL int
#define TRUE 1
#define FALSE 0

BOOL Initialize(void);

void Dispose(void);

void EnableMute(BOOL bEnabled);

BOOL IsMuteEnabled();


#ifdef __cplusplus
}
#endif                          /* __cplusplus */


#endif /* MUTERPULSEAUDIO_H_ */
