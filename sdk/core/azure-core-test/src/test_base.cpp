// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include "azure/core/test/test_base.hpp"

#include <azure/core/internal/json/json.hpp>

#include <fstream>
#include <iostream>

using namespace Azure::Core::Json::_internal;

void Azure::Core::Test::TestBase::TearDown()
{
  if (m_wasSkipped || m_testContext.IsLiveMode())
  {
    return;
  }
  if (m_testProxy)
  {
    if (m_testProxy->IsRecordMode())
    {
      m_testProxy->StopPlaybackRecord(TestMode::RECORD);
    }
    if (m_testProxy->IsPlaybackMode())
    {
      m_testProxy->StopPlaybackRecord(TestMode::PLAYBACK);
    }
  }
}


