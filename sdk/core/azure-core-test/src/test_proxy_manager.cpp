// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include <azure/core/internal/environment.hpp>
#include <azure/core/internal/json/json.hpp>
#include <azure/core/internal/strings.hpp>

#include "azure/core/test/test_proxy_manager.hpp"

#include <fstream>
#include <iostream>
#include <regex>
#include <stdexcept>
#include <string>

using namespace Azure::Core::Test;
using namespace Azure::Core;
using Azure::Core::_internal::Environment;

TestMode TestProxyManager::GetTestMode()
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
  throw std::runtime_error("Invalid environment variable value for AZURE_TEST_MODE: " + value);
}

void TestProxyManager::ConfigureInsecureConnection(
    Azure::Core::_internal::ClientOptions& clientOptions)
{
  // NOTE: perf-fm is injecting the SSL config and transport here for the client options
  //       If the test overrides the options/transport, this can be undone.
  if (m_isInsecureEnabled)
  {
#if defined(BUILD_CURL_HTTP_TRANSPORT_ADAPTER)
    Azure::Core::Http::CurlTransportOptions curlOptions;
    curlOptions.SslVerifyPeer = false;
    curlOptions.SslOptions.AllowFailedCrlRetrieval = true;
    clientOptions.Transport.Transport
        = std::make_shared<Azure::Core::Http::CurlTransport>(curlOptions);
#elif defined(BUILD_TRANSPORT_WINHTTP_ADAPTER)
    Azure::Core::Http::WinHttpTransportOptions winHttpOptions;
    winHttpOptions.IgnoreUnknownCertificateAuthority = true;
    clientOptions.Transport.Transport
        = std::make_shared<Azure::Core::Http::WinHttpTransport>(winHttpOptions);
#else
    // avoid the variable not used warning
    (void)clientOptions;
#endif
  }
}

std::string TestProxyManager::PrepareRequestBody()
{
  std::string recordingPath = m_testContext.GetTestRecordingPathName();
  std::string body;

  auto sdkPos = recordingPath.rfind("sdk");
  recordingPath = recordingPath.substr(sdkPos, recordingPath.size() - sdkPos);
  body = "{\"x-recording-file\":\"";
  body.append(recordingPath);
  body.append("\"");
  body.append(",");
  body.append("\"x-recording-assets-file\":\"");
  body.append(m_testContext.AssetsPath);
  body.append("\"");

  body.append("}");

  return body;
}

void TestProxyManager::StartPlaybackRecord(TestMode testMode)
{
  if (IsPlaybackMode() || IsRecordMode())
  {
    std::string mode = (IsPlaybackMode() ? "playback" : "record");
    return;
  }

  m_currentMode = testMode;

  Azure::Core::Url startRequest(m_proxy);
  if (testMode == TestMode::PLAYBACK)
  {
    startRequest.AppendPath("playback");
  }
  else if (testMode == TestMode::RECORD)
  {
    startRequest.AppendPath("record");
  }

  startRequest.AppendPath("start");
  std::string body = PrepareRequestBody();

  Azure::Core::IO::MemoryBodyStream payloadStream(
      reinterpret_cast<const uint8_t*>(body.data()), body.size());
  Azure::Core::Http::Request request(
      Azure::Core::Http::HttpMethod::Post, startRequest, &payloadStream);

  Azure::Core::Context ctx;
  auto response = m_privatePipeline->Send(request, ctx);

  auto const& headers = response->GetHeaders();
  auto findHeader = std::find_if(
      headers.begin(), headers.end(), [](std::pair<std::string const&, std::string const&> h) {
        return h.first == "x-recording-id";
      });
  m_testContext.RecordingId = findHeader->second;
}

void TestProxyManager::StopPlaybackRecord(TestMode testMode)
{
  if (testMode == TestMode::PLAYBACK && !IsPlaybackMode())
  {
    throw std::runtime_error("TestProxy not in playback mode.");
  }
  if (testMode == TestMode::RECORD && !IsRecordMode())
  {
    throw std::runtime_error("TestProxy not in record mode");
  }

  Azure::Core::Url stopRequest(m_proxy);

  if (m_currentMode == TestMode::PLAYBACK)
  {
    stopRequest.AppendPath("playback");
  }
  else if (m_currentMode == TestMode::RECORD)
  {
    stopRequest.AppendPath("record");
  }

  stopRequest.AppendPath("stop");

  Azure::Core::Http::Request request(Azure::Core::Http::HttpMethod::Post, stopRequest);
  request.SetHeader("x-recording-id", m_testContext.RecordingId);
  Azure::Core::Context ctx;

  m_privatePipeline->Send(request, ctx);

  m_testContext.RecordingId.clear();
  m_currentMode = TestMode::LIVE;
}
std::unique_ptr<Azure::Core::Http::Policies::HttpPolicy> TestProxyManager::GetTestProxyPolicy()
{
  return std::make_unique<Azure::Core::Test::TestProxyPolicy>(this);
}

bool TestProxyManager::CheckSanitizers()
{
  Azure::Core::Url checkRequest(m_proxy);

  checkRequest.AppendPath("Info");
  checkRequest.AppendPath("Active");

  {
    Azure::Core::Http::Request request(Azure::Core::Http::HttpMethod::Get, checkRequest);
    Azure::Core::Context ctx;
    auto response = m_privatePipeline->Send(request, ctx);

    auto rawResponse = response->GetBody();
    std::string stringBody(rawResponse.begin(), rawResponse.end());
    std::string regex = "\"https://(?<account>[a-zA-Z0-9\\-]+).\"";
    std::vector<std::string> stringsInOrder
        = {"UriRegexSanitizer",
           regex,
           "BodyRegexSanitizer",
           regex,
           "HeaderRegexSanitizer",
           regex,
           "GeneralRegexSanitizer",
           regex,
           "CustomDefaultMatcher"};

    size_t position = 0;

    for (auto& part : stringsInOrder)
    {
      position = stringBody.find(part, position);
      if (position == std::string::npos)
      {
        return false;
      }
    }
  }

  return true;
}

void TestProxyManager::SetProxySanitizer()
{
  if (CheckSanitizers())
  {
    return;
  }
  Azure::Core::Url sanitizerRequest(m_proxy);
  sanitizerRequest.AppendPath("Admin");
  sanitizerRequest.AppendPath("AddSanitizer");
  const std::string regexBody
      = "{\"key\" : \"Location\",\"value\" : \"REDACTED\",\"regex\": "
        "\"https://(?<account>[a-zA-Z0-9\\\\-]+).\",\"groupForReplace\" : \"account\"}";

  Azure::Core::Url matcherRequest(m_proxy);
  matcherRequest.AppendPath("Admin");
  matcherRequest.AppendPath("SetMatcher");
  std::string matcherBody;
  {
    auto jsonRoot = Json::_internal::json::object();
    jsonRoot["compareBodies"] = false;
    jsonRoot["ignoreQueryOrdering"] = true;
    const std::vector<std::string> excludedHeaders = {
        "Expect",
        "Connection",
    };
    jsonRoot["excludedHeaders"] = std::accumulate(
        excludedHeaders.begin(),
        excludedHeaders.end(),
        std::string(),
        [](const std::string& lhs, const std::string& rhs) {
          return lhs + (lhs.empty() ? "" : ",") + rhs;
        });
    const std::vector<std::string> ignoredHeaders = {
        "x-ms-copy-source",
        "x-ms-file-change-time",
        "x-ms-file-creation-time",
        "x-ms-file-last-write-time",
        "x-ms-rename-source",
    };
    const std::vector<std::string> ignoreQueryParameters = {
        "st",
        "se",
        "sig",
    };
    jsonRoot["ignoredHeaders"] = std::accumulate(
        ignoredHeaders.begin(),
        ignoredHeaders.end(),
        std::string(),
        [](const std::string& lhs, const std::string& rhs) {
          return lhs + (lhs.empty() ? "" : ",") + rhs;
        });
    jsonRoot["ignoredQueryParameters"] = std::accumulate(
        ignoreQueryParameters.begin(),
        ignoreQueryParameters.end(),
        std::string(),
        [](const std::string& lhs, const std::string& rhs) {
          return lhs + (lhs.empty() ? "" : ",") + rhs;
        });
    matcherBody = jsonRoot.dump();
  }

  {
    Azure::Core::IO::MemoryBodyStream payloadStream(
        reinterpret_cast<const uint8_t*>(regexBody.data()), regexBody.size());
    Azure::Core::Http::Request request(
        Azure::Core::Http::HttpMethod::Post, sanitizerRequest, &payloadStream);
    request.SetHeader("x-abstraction-identifier", "UriRegexSanitizer");
    Azure::Core::Context ctx;
    auto response = m_privatePipeline->Send(request, ctx);
  }
  {
    Azure::Core::IO::MemoryBodyStream payloadStream(
        reinterpret_cast<const uint8_t*>(regexBody.data()), regexBody.size());
    Azure::Core::Http::Request request(
        Azure::Core::Http::HttpMethod::Post, sanitizerRequest, &payloadStream);
    request.SetHeader("x-abstraction-identifier", "BodyRegexSanitizer");
    Azure::Core::Context ctx;
    auto response = m_privatePipeline->Send(request, ctx);
  }
  {
    Azure::Core::IO::MemoryBodyStream payloadStream(
        reinterpret_cast<const uint8_t*>(regexBody.data()), regexBody.size());
    Azure::Core::Http::Request request(
        Azure::Core::Http::HttpMethod::Post, sanitizerRequest, &payloadStream);
    request.SetHeader("x-abstraction-identifier", "HeaderRegexSanitizer");
    Azure::Core::Context ctx;
    auto response = m_privatePipeline->Send(request, ctx);
  }
  {
    Azure::Core::IO::MemoryBodyStream payloadStream(
        reinterpret_cast<const uint8_t*>(regexBody.data()), regexBody.size());
    Azure::Core::Http::Request request(
        Azure::Core::Http::HttpMethod::Post, sanitizerRequest, &payloadStream);
    request.SetHeader("x-abstraction-identifier", "GeneralRegexSanitizer");
    Azure::Core::Context ctx;
    auto response = m_privatePipeline->Send(request, ctx);
  }
  {
    Azure::Core::IO::MemoryBodyStream payloadStream(
        reinterpret_cast<const uint8_t*>(matcherBody.data()), matcherBody.size());
    Azure::Core::Http::Request request(
        Azure::Core::Http::HttpMethod::Post, matcherRequest, &payloadStream);
    request.SetHeader("x-abstraction-identifier", "CustomDefaultMatcher");
    Azure::Core::Context ctx;
    auto response = m_privatePipeline->Send(request, ctx);
  }
}
