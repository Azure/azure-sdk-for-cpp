// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include <azure/core/internal/json/json.hpp>

#include "azure/core/test/network_models.hpp"
#include "azure/core/test/test_base.hpp"

#include <fstream>

Azure::Core::Test::TestMode Azure::Core::Test::TestBase::testMode;

using namespace Azure::Core::Json::_internal;

void Azure::Core::Test::TestBase::TearDown()
{
  json root;
  json records;
  auto const& recordData = m_interceptor.GetRecordedData();
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
  outFile.open(m_testContext.GetTestName() + ".json");
  outFile << root.dump(2);
  outFile.close();
}
