/*
 * SinkInput.h
 *
 *  Created on: 2012-1-3
 *      Author: Yuan Xulei
 */

#ifndef SINKINPUT_H_
#define SINKINPUT_H_

#define INVALID_INDEX 0xffffffff

#include <pulse/pulseaudio.h>

class SinkInput
{
public:
  SinkInput();
  virtual ~SinkInput();

  void UpdateData(const pa_sink_input_info* info);

  uint32_t GetIndex() const;
  char *GetProcessBinary() const;
  char *GetApplicationName() const {return m_applicationName;}
  int GetProcessId() const;
  bool IsMute() const;

  void SetMute(pa_context* context, bool mute);

private:
  static char* DupPaPopString(pa_proplist *list, const char* popName);

private:
  uint32_t m_index;
  char* m_processBinary;
  char* m_applicationName;
  int m_processId;
  bool m_mute;
};

#endif /* SINKINPUT_H_ */
