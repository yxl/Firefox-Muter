/*
 * AudioMixerControl.h
 *
 *  Created on: 2012-1-2
 *      Author: Yuan Xulei
 */

#ifndef AUDIOMIXERCONTROL_H_
#define AUDIOMIXERCONTROL_H_

#include <pulse/pulseaudio.h>
#include <pthread.h>
#include <map>

class SinkInput;

class AudioMixerControl
{
public:
  AudioMixerControl();
  virtual
  ~AudioMixerControl();

  bool Open();
  bool Close();

  void EnableMute(bool mute);

  bool IsMute() const {return m_mute;}

private:
  void NewPaContext();
  void UpdateMuteStatus();

  static void PaContextStateCallback(pa_context *context, void *userdata);
  static void PaContextSubscribeCallback(pa_context *context,
      pa_subscription_event_type_t t, uint32_t index, void *userdata);
  static void PaContextGetSinkInputInfoCallback(pa_context *context,
      const pa_sink_input_info *i, int eol, void *userdata);

  void OnPaContextReady();
  void OnPaContextFailed();

  void ReqUpdateSinkInputInfo(uint32_t index);
  void UpdateSinkInput(const pa_sink_input_info *info);
  void RemoveSinkInput(uint32_t index);
  void RemoveAllInputSinks();

  static void* LoopThread(void *userdata);
  void FreeThread();
private:
  pa_mainloop* m_pa_mainloop;
  pa_mainloop_api* m_pa_api;
  pa_context* m_pa_context;
  std::map<uint32_t, SinkInput*> m_sinkInputs;

  bool m_mute;

  pthread_t m_threadId;
  bool m_threadAborting;
  bool m_threadStopped;

};

#endif /* AUDIOMIXERCONTROL_H_ */
