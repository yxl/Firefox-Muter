/*
 * SinkInput.cpp
 *
 *  Created on: 2012-1-3
 *      Author: Yuan Xulei
 */

#include "SinkInput.h"
#include <string.h>

SinkInput::SinkInput() :
    m_index(INVALID_INDEX), m_processBinary(NULL), m_processId(-1), m_mute(
        false)
{
}

void SinkInput::UpdateData(const pa_sink_input_info *info)
{
  if (info != NULL)
  {
    m_index = info->index;

    m_processBinary = DupPaPopString(info->proplist,
        PA_PROP_APPLICATION_PROCESS_BINARY);

    char *szProcessId = DupPaPopString(info->proplist,
        PA_PROP_APPLICATION_PROCESS_ID);
    if (szProcessId)
    {
      m_processId = atoi(szProcessId);
      free(szProcessId);
    }

    m_mute = info->mute != 0;
  }
}

void SinkInput::SetMute(pa_context* context, bool mute)
{
  pa_operation *o;

  o = pa_context_set_sink_input_mute(context, m_index, mute ? 1 : 0, NULL,
      NULL);

  if (o == NULL)
  {
    return;
  }

  pa_operation_unref(o);
}

bool SinkInput::IsMute() const
{
  return m_mute;
}

char *SinkInput::DupPaPopString(pa_proplist *list, const char *popName)
{
  char* result = NULL;

  const void *data;
  size_t nbytes = 0;
  pa_proplist_get(list, popName, &data, &nbytes);
  if (nbytes > 0)
  {
    result = strndup(reinterpret_cast<const char *>(data), nbytes);
  }
  return result;
}

SinkInput::~SinkInput()
{
  // Do not use "delete" to free the memory, as the memory is allocated by "malloc".
  if (m_processBinary)
  {
    free(m_processBinary);
  }
}

uint32_t SinkInput::GetIndex() const
{
  return m_index;
}

char *SinkInput::GetProcessBinary() const
{
  return m_processBinary;
}

int SinkInput::GetProcessId() const
{
  return m_processId;
}

