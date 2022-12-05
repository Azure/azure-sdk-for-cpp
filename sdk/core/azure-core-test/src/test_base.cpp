// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include <azure/core/internal/json/json.hpp>

#include "azure/core/test/interceptor_manager.hpp"
#include "azure/core/test/network_models.hpp"
#include "azure/core/test/test_base.hpp"

#include <fstream>
#include <iostream>

using namespace Azure::Core::Json::_internal;

void Azure::Core::Test::TestBase::TearDown()
{

  if (m_testContext.IsLiveMode() || m_testContext.IsPlaybackMode())
  {
    // Don't want to record here
    return;
  }

  json root;
  json records;
  auto const& recordData = m_interceptor->GetRecordedData();

  if (recordData.NetworkCallRecords.size() == 0)
  {
    // Don't make empty recordings
    return;
  }

  for (auto const& record : recordData.NetworkCallRecords)
  {
    json recordJson;
    recordJson["Headers"] = json(record.Headers);
    recordJson["Response"] = json(record.Response);
    recordJson["Method"] = record.Method;
    recordJson["Url"] = record.Url;
    records.push_back(recordJson);
  }
  root["networkCallRecords"] = records;

  // Write json to file
  std::ofstream outFile;
  // AZURE_TEST_RECORDING_DIR is exported from CMAKE
  std::string testPath(m_testContext.RecordingPath);
  outFile.open(testPath + "/" + m_testContext.GetTestPlaybackRecordingName() + ".json");
  outFile << root.dump(2) << std::endl;
  outFile.close();
}