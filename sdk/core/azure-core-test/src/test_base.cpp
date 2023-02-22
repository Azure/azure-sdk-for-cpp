// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include <azure/core/internal/json/json.hpp>

#include "azure/core/test/test_base.hpp"

#include <fstream>
#include <iostream>

using namespace Azure::Core::Json::_internal;

void Azure::Core::Test::TestBase::TearDown()
{
  if (m_wasSkipped || m_testContext.IsLiveMode())
  {
    return;
  }
  if (m_testProxy->IsRecordMode())
  {
    m_testProxy->StopPlaybackRecord(TestMode::RECORD);
  }
  if (m_testProxy->IsPlaybackMode())
  {
    m_testProxy->StopPlaybackRecord(TestMode::PLAYBACK);
  }
}
