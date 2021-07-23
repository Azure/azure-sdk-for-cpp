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
  auto records = root["networkCallRecords"];
  auto const& recordData = m_interceptor.GetRecordedData();

  for (auto const& record : recordData.NetworkCallRecords)
  {
    json recordJson;
    recordJson["Method"] = record.Method;
    recordJson["Url"] = record.Url;
    auto headersRecords = recordJson["Headers"];
    for (auto const& requestHeader : record.Headers)
    {
      headersRecords[requestHeader.first] = requestHeader.second;
    }
    auto responseRecords = recordJson["Response"];
    for (auto const& responseFragment : record.Response)
    {
      responseRecords[responseFragment.first] = responseFragment.second;
    }
    records.push_back(recordJson);
  }

  // Write json to file
  std::ofstream outFile;
  outFile.open(m_testContext.GetTestName() + ".json");
  outFile << root.dump();
  outFile.close();
}
