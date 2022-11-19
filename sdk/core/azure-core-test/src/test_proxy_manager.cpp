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
  throw std::runtime_error("Invalid environment variable: " + value);
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
  std::string body = "{\"x-recording-file\":\"";
  body.append(m_testContext.GetTestRecordingPathName());
  body.append("\"}");

  return body;
}

void TestProxyManager::SetStartRecordMode()
{
  if (IsPlaybackMode() || IsRecordMode())
  {
    std::string mode = (IsPlaybackMode() ? "playback" : "record");
    // throw std::runtime_error();
    std::cout << "TestProxy already in " + mode + " mode.";
    return;
  }

  m_currentMode = TestMode::RECORD;
  StartPlaybackRecord(m_currentMode);
}

void TestProxyManager::SetStartPlaybackMode()
{
  if (IsPlaybackMode() || IsRecordMode())
  {
    std::string mode = (IsPlaybackMode() ? "playback" : "record");
    // throw std::runtime_error("TestProxy already in " + mode + " mode. First stop current mode.");
    std::cout << "TestProxy already in " + mode + " mode.";
    return;
  }

  m_currentMode = TestMode::PLAYBACK;
  StartPlaybackRecord(m_currentMode);
}

void TestProxyManager::StartPlaybackRecord(TestMode testMode)
{
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

void TestProxyManager::SetStopPlaybackMode()
{
  if (!IsPlaybackMode())
  {
    throw std::runtime_error("TestProxy not in playback mode.");
  }
  StopPlaybackRecord();
  m_currentMode = TestMode::LIVE;
}

void TestProxyManager::SetStopRecordMode()
{
  if (!IsRecordMode())
  {
    throw std::runtime_error("TestProxy not in record mode");
  }
  StopPlaybackRecord();
  m_currentMode = TestMode::LIVE;
}

void TestProxyManager::StopPlaybackRecord()
{
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
}
std::unique_ptr<Azure::Core::Http::Policies::HttpPolicy> TestProxyManager::GetTestProxyPolicy()
{
  return std::make_unique<Azure::Core::Test::TestProxyPolicy>(this);
}

void TestProxyManager::SetProxySanitizer()
{
  Azure::Core::Url sanitizerRequest(m_proxy);

  sanitizerRequest.AppendPath("Admin");
  sanitizerRequest.AppendPath("AddSanitizer");
  {

    std::string body = "{\"key\" : \"Location\",\"value\" : \"REDACTED\",\"regex\": "
                       "\"https://(?<account>[a-z]+).*\",\"groupForReplace\" : \"account\"}";

    Azure::Core::IO::MemoryBodyStream payloadStream(
        reinterpret_cast<const uint8_t*>(body.data()), body.size());
    Azure::Core::Http::Request request(
        Azure::Core::Http::HttpMethod::Post, sanitizerRequest, &payloadStream);
    request.SetHeader("x-abstraction-identifier", "UriRegexSanitizer");
    Azure::Core::Context ctx;
    auto response = m_privatePipeline->Send(request, ctx);
  }
  {

    std::string body = "{\"key\" : \"Location\",\"value\" : \"REDACTED\",\"regex\": "
                       "\"https://(?<account>[a-z]+).*\",\"groupForReplace\" : \"account\"}";

    Azure::Core::IO::MemoryBodyStream payloadStream(
        reinterpret_cast<const uint8_t*>(body.data()), body.size());
    Azure::Core::Http::Request request(
        Azure::Core::Http::HttpMethod::Post, sanitizerRequest, &payloadStream);
    request.SetHeader("x-abstraction-identifier", "BodyRegexSanitizer");
    Azure::Core::Context ctx;
    auto response = m_privatePipeline->Send(request, ctx);
  }
}
