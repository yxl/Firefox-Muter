/*
 * AudioMixerControl.cpp
 *
 *  Created on: 2012-1-2
 *      Author: Yuan Xulei
 */
#include "../config.h"
#include "AudioMixerControl.h"
#include "SinkInput.h"

#include <string.h>
#include <assert.h>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>

AudioMixerControl::AudioMixerControl() :
    m_pa_api(NULL), m_pa_context(NULL), m_pa_mainloop(NULL), m_mute(false), m_threadAborting(
        false), m_threadStopped(true), m_threadId(0)
{
  // Create a mainloop API
  m_pa_mainloop = pa_mainloop_new();
  assert(m_pa_mainloop != NULL);
  m_pa_api = pa_mainloop_get_api(m_pa_mainloop);
  assert(m_pa_api != NULL);
  NewPaContext();
}

AudioMixerControl::~AudioMixerControl()
{
  RemoveAllInputSinks();
  // Free mainloop
  pa_mainloop_free(m_pa_mainloop);
}

bool AudioMixerControl::Open()
{
  if (m_pa_context == NULL)
  {
    return false;
  }

  // Check if it is already connected?
  if (pa_context_get_state(m_pa_context) != PA_CONTEXT_UNCONNECTED)
  {
    return false;
  }

  pa_context_set_state_callback(m_pa_context,
      AudioMixerControl::PaContextStateCallback, static_cast<void *>(this));

  int res = pa_context_connect(m_pa_context, NULL,
      (pa_context_flags_t) PA_CONTEXT_NOFAIL, NULL);

  if (res < 0)
  {
    printf("[%s] AudioMixerControl::Open Failed to connect\n", PACKAGE_NAME);
    return false;
  }

  if (m_threadStopped)
  {
    return InitThread();
  }
  return true;
}

bool AudioMixerControl::InitThread()
{
  m_threadAborting = false;
  int res = pthread_create(&m_threadId, NULL, AudioMixerControl::LoopThread,
      static_cast<void *>(this));
  if (res != 0)
  {
    printf("[%s] AudioMixerControl::InitThread failed\n", PACKAGE_NAME);
    return false;
  }
  return true;
}

void AudioMixerControl::FreeThread()
{
  m_threadAborting = true;
  if (m_threadId != 0)
  {
    pa_mainloop_quit(m_pa_mainloop, 0);
    int res = pthread_join(m_threadId, NULL);
    if (res == 0)
    {
      m_threadId = 0;
    }
  }
}

void AudioMixerControl::EnableMute(bool mute)
{
  m_mute = mute;
  UpdateMuteStatus();
}

bool AudioMixerControl::Close()
{
  if (m_pa_context == NULL)
  {
    return false;
  }

  FreeThread();

  pa_context_disconnect(m_pa_context);

  return true;
}

void AudioMixerControl::PaContextStateCallback(pa_context *context,
    void *userdata)
{
  AudioMixerControl* pThis = static_cast<AudioMixerControl *>(userdata);
  switch (pa_context_get_state(context))
  {
  case PA_CONTEXT_UNCONNECTED:
  case PA_CONTEXT_CONNECTING:
  case PA_CONTEXT_AUTHORIZING:
  case PA_CONTEXT_SETTING_NAME:
    break;

  case PA_CONTEXT_READY:
    pThis->OnPaContextReady();
    break;

  case PA_CONTEXT_FAILED:
    pThis->OnPaContextFailed();
    break;
  case PA_CONTEXT_TERMINATED:
  default:
    break;
  }

  printf("[%s] AudioMixerControl::PaContextStateCallback\n", PACKAGE_NAME);
}

void AudioMixerControl::NewPaContext()
{
  if (m_pa_context != NULL)
  {
    return;
  }

  pa_proplist *proplist = pa_proplist_new();

  pa_proplist_sets(proplist, PA_PROP_APPLICATION_NAME, PACKAGE);
  pa_proplist_sets(proplist, PA_PROP_APPLICATION_ID, "name.yxl.muter");
  pa_proplist_sets(proplist, PA_PROP_APPLICATION_ICON_NAME, "Firefox-Muter");
  pa_proplist_sets(proplist, PA_PROP_APPLICATION_VERSION, PACKAGE_VERSION);

  m_pa_context = pa_context_new_with_proplist(m_pa_api, NULL, proplist);

  pa_proplist_free(proplist);
  assert(m_pa_context != NULL);
}

void AudioMixerControl::OnPaContextReady()
{
  pa_operation *o;

  pa_context_set_subscribe_callback(m_pa_context,
      AudioMixerControl::PaContextSubscribeCallback, static_cast<void *>(this));
  o = pa_context_subscribe(
      m_pa_context,
      (pa_subscription_mask_t) (PA_SUBSCRIPTION_MASK_SINK
          | PA_SUBSCRIPTION_MASK_SINK_INPUT | PA_SUBSCRIPTION_MASK_CLIENT
          | PA_SUBSCRIPTION_MASK_SERVER | PA_SUBSCRIPTION_MASK_CARD), NULL,
      NULL);

  if (o == NULL)
  {
    printf(
        "[%s] AudioMixerControl::OnPaContextReady a_context_subscribe() failed\n",
        PACKAGE_NAME);
    return;
  }
  pa_operation_unref(o);

  ReqUpdateSinkInputInfo(INVALID_INDEX);
}

void AudioMixerControl::OnPaContextFailed()
{
  if (m_pa_context != NULL)
  {
    pa_context_unref(m_pa_context);
    m_pa_context = NULL;
    NewPaContext();
  }

  RemoveAllInputSinks();

  Open();
}

void AudioMixerControl::PaContextSubscribeCallback(pa_context *context,
    pa_subscription_event_type_t t, uint32_t index, void *userdata)
{
  AudioMixerControl* pThis = static_cast<AudioMixerControl *>(userdata);
  switch (t & PA_SUBSCRIPTION_EVENT_FACILITY_MASK)
  {
  case PA_SUBSCRIPTION_EVENT_SINK_INPUT:
    if ((t & PA_SUBSCRIPTION_EVENT_TYPE_MASK) == PA_SUBSCRIPTION_EVENT_REMOVE)
    {
      pThis->RemoveSinkInput(index);
    }
    else
    {
      pThis->ReqUpdateSinkInputInfo(index);
    }
    break;
  }
}

void AudioMixerControl::ReqUpdateSinkInputInfo(uint32_t index)
{
  pa_operation *o;

  if (index == INVALID_INDEX)
  {
    o = pa_context_get_sink_input_info_list(m_pa_context,
        AudioMixerControl::PaContextGetSinkInputInfoCallback,
        static_cast<void *>(this));
  }
  else
  {
    o = pa_context_get_sink_input_info(m_pa_context, index,
        AudioMixerControl::PaContextGetSinkInputInfoCallback,
        static_cast<void *>(this));
  }

  if (o == NULL)
  {
    printf(
        "[%s] AudioMixerControl::ReqUpdateSinkInputInfo pa_context_get_sink_input_info_list() / pa_context_get_sink_input_info() failed\n",
        PACKAGE_NAME);
    return;
  }
  pa_operation_unref(o);
}

void AudioMixerControl::PaContextGetSinkInputInfoCallback(pa_context *context,
    const pa_sink_input_info *i, int eol, void *userdata)
{
  AudioMixerControl* pThis = static_cast<AudioMixerControl *>(userdata);
  if (eol == 0)
  {
    pThis->UpdateSinkInput(i);
  }
}

static int GetParentProcessId(int processId)
{
  char pname[100];
  char buffer[666];
  snprintf(pname, sizeof(pname), "/proc/%d/stat", processId);

  int procfd;

  int ppid = -1;

  if ((procfd = open(pname, O_RDONLY)) != -1
      && read(procfd, buffer, sizeof(buffer)) > 0)
  {
    int pid;
    char fname[100];
    char state;
    sscanf(buffer, "%d %s %c %d", &pid, fname, &state, &ppid);
  }

  close(procfd);

  return ppid;
}

void AudioMixerControl::UpdateSinkInput(const pa_sink_input_info *info)
{
  printf("[%s] AudioMixerControl::UpdateSinkInput\n", PACKAGE_NAME);

  SinkInput* stream = NULL;
  std::map<uint32_t, SinkInput*>::iterator iter = m_sinkInputs.find(
      info->index);
  if (iter != m_sinkInputs.end())
  {
    stream = iter->second;
    stream->UpdateData(info);
  }
  else
  {
    stream = new SinkInput();
    stream->UpdateData(info);

    // Check if the stream belongs to Firefox
    int pid = getpid();
    if (stream->GetProcessId() != pid
        && GetParentProcessId(stream->GetProcessId()) != pid
        && (stream->GetApplicationName() == NULL
            || (strcmp(stream->GetApplicationName(), "Movie browser plugin")
                != 0 && strstr(stream->GetApplicationName(), "plug-in") == NULL
                && strstr(stream->GetProcessBinary(), "plugin-container")
                    == NULL)))
    {
      delete stream;
      return;
    }

    m_sinkInputs.insert(std::make_pair(info->index, stream));
    printf("[%s] Add SinkInput\n", PACKAGE_NAME);
    UpdateMuteStatus();
  }
}

void AudioMixerControl::RemoveSinkInput(uint32_t index)
{
  printf("[%s] AudioMixerControl::RemoveSinkInput\n", PACKAGE_NAME);

  SinkInput *stream = NULL;

  std::map<uint32_t, SinkInput*>::iterator iter = m_sinkInputs.find(index);
  if (iter != m_sinkInputs.end())
  {
    stream = iter->second;
    delete stream;
    m_sinkInputs.erase(index);
    printf("[%s] Erase SinkInput\n", PACKAGE_NAME);
    UpdateMuteStatus();
  }
}

void AudioMixerControl::RemoveAllInputSinks()
{
  while (m_sinkInputs.size() > 0)
  {
    std::map<uint32_t, SinkInput*>::iterator iter = m_sinkInputs.begin();
    SinkInput *stream = iter->second;
    delete stream;
    m_sinkInputs.erase(iter);
  }
}

void AudioMixerControl::UpdateMuteStatus()
{
  for (std::map<uint32_t, SinkInput*>::iterator iter = m_sinkInputs.begin();
      iter != m_sinkInputs.end(); iter++)
  {
    SinkInput *stream = iter->second;
    if (stream->IsMute() != m_mute)
    {
      stream->SetMute(m_pa_context, m_mute);
    }
  }
}

void* AudioMixerControl::LoopThread(void *userdata)
{
  AudioMixerControl* pThis = static_cast<AudioMixerControl *>(userdata);

  pThis->m_threadStopped = false;

  int retval = 0;
  while (!pThis->m_threadAborting
      && pa_mainloop_iterate(pThis->m_pa_mainloop, 1, &retval) >= 0)
  {
  }

  pthread_exit(0);

  pThis->m_threadStopped = true;

  return NULL;
}

