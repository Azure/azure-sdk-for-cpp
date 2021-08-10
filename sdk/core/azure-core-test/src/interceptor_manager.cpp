// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#ifdef _MSC_VER
#define _CRT_SECURE_NO_WARNINGS // for std::getenv()
#endif

#include <azure/core/internal/json/json.hpp>
#include <azure/core/internal/strings.hpp>

#include "azure/core/test/interceptor_manager.hpp"
#include "private/environment.hpp"

#include <fstream>
#include <iostream>
#include <stdexcept>
#include <string>

using namespace Azure::Core::Test;
using namespace Azure::Core::Json::_internal;
using namespace Azure::Core;

TestMode InterceptorManager::GetTestMode() { return _detail::Environment::GetTestMode(); }

void InterceptorManager::LoadTestData()
{
  if (m_recordedData.NetworkCallRecords.size() > 0)
  {
    // TestData was loaded it before.
    return;
  }

  std::string const recordingName(
      m_testContext.RecordingPath + "/" + m_testContext.GetTestPlaybackRecordingName() + ".json");
  std::ifstream readFile(recordingName);
  if (!readFile.is_open())
  {
    throw std::runtime_error("Can't open recording: " + recordingName);
  }

  std::string recordingContent(
      (std::istreambuf_iterator<char>(readFile)), std::istreambuf_iterator<char>());

  auto const jsonRecord = json::parse(recordingContent);
  auto const networkRecords = jsonRecord["networkCallRecords"];
  for (auto const& record : networkRecords)
  {
    NetworkCallRecord modelRecord;
    modelRecord.Method = record["Method"];
    modelRecord.Url = record["Url"];
    modelRecord.Headers = record["Headers"].get<std::map<std::string, std::string>>();
    modelRecord.Response = record["Response"].get<std::map<std::string, std::string>>();
    m_recordedData.NetworkCallRecords.push_back(modelRecord);
  }
  readFile.close();
}

Url InterceptorManager::RedactUrl(Url const& url)
{
  Azure::Core::Url redactedUrl;
  redactedUrl.SetScheme(url.GetScheme());
  auto host = url.GetHost();
  auto hostWithNoAccount = std::find(host.begin(), host.end(), '.');
  redactedUrl.SetHost("REDACTED" + std::string(hostWithNoAccount, host.end()));
  redactedUrl.SetPath(url.GetPath());
  // Query parameters
  for (auto const& qp : url.GetQueryParameters())
  {
    if (qp.first == "sig")
    {
      redactedUrl.AppendQueryParameter("sig", "REDACTED");
    }
    else
    {
      redactedUrl.AppendQueryParameter(qp.first, qp.second);
    }
  }
  return redactedUrl;
}
