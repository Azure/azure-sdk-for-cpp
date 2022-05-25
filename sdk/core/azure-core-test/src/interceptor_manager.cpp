// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include <azure/core/internal/environment.hpp>
#include <azure/core/internal/json/json.hpp>
#include <azure/core/internal/request_sanitizer.hpp>
#include <azure/core/internal/strings.hpp>

#include "azure/core/test/interceptor_manager.hpp"

#include <fstream>
#include <iostream>
#include <regex>
#include <stdexcept>
#include <string>

using namespace Azure::Core::Test;
using namespace Azure::Core::Json::_internal;
using namespace Azure::Core;
using Azure::Core::_internal::Environment;

TestMode InterceptorManager::GetTestMode()
{
  auto value = Environment::GetVariable("AZURE_TEST_MODE");
  if (value.empty())
  {
    return Azure::Core::Test::TestMode::LIVE;
  }

  if (Azure::Core::_internal::StringExtensions::LocaleInvariantCaseInsensitiveEqual(
          value, "RECORD"))
  {
    return Azure::Core::Test::TestMode::RECORD;
  }
  else if (Azure::Core::_internal::StringExtensions::LocaleInvariantCaseInsensitiveEqual(
               value, "PLAYBACK"))
  {
    return Azure::Core::Test::TestMode::PLAYBACK;
  }
  else if (Azure::Core::_internal::StringExtensions::LocaleInvariantCaseInsensitiveEqual(
               value, "LIVE"))
  {
    return Azure::Core::Test::TestMode::LIVE;
  }

  // unexpected variable value
  throw std::runtime_error("Invalid environment variable: " + value);
}

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
  return Azure::Core::_internal::InputSanitizer::SanitizeUrl(url);
}
