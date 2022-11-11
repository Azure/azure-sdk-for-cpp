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
  Azure::Core::Url startPlayback(m_proxy);
  startPlayback.AppendPath("record");
  startPlayback.AppendPath("start");
  std::string body = PrepareRequestBody();

  Azure::Core::IO::MemoryBodyStream payloadStream(
      reinterpret_cast<const uint8_t*>(body.data()), body.size());
  Azure::Core::Http::Request request(
      Azure::Core::Http::HttpMethod::Post, startPlayback, &payloadStream);

  Azure::Core::Context ctx;
  auto response = m_privatePipeline->Send(request, ctx);

  auto const& headers = response->GetHeaders();
  auto findHeader = std::find_if(
      headers.begin(), headers.end(), [](std::pair<std::string const&, std::string const&> h) {
        return h.first == "x-recording-id";
      });
  m_testContext.RecordingId = findHeader->second;
  m_isRecordMode = true;
}

void TestProxyManager::SetStopRecordMode()
{
  Azure::Core::Url startPlayback(m_proxy);
  startPlayback.AppendPath("record");
  startPlayback.AppendPath("stop");
  Azure::Core::Http::Request request(Azure::Core::Http::HttpMethod::Post, startPlayback);

  request.SetHeader("x-recording-id", m_testContext.RecordingId);

  Azure::Core::Context ctx;
  auto response = m_privatePipeline->Send(request, ctx);

  m_isRecordMode = false;
  m_testContext.RecordingId.clear();
}

void TestProxyManager::SetStartPlaybackMode()
{
  Azure::Core::Url startPlayback(m_proxy);
  startPlayback.AppendPath("playback");
  startPlayback.AppendPath("start");
  std::string body = PrepareRequestBody();

  Azure::Core::IO::MemoryBodyStream payloadStream(
      reinterpret_cast<const uint8_t*>(body.data()), body.size());
  Azure::Core::Http::Request request(
      Azure::Core::Http::HttpMethod::Post, startPlayback, &payloadStream);
  
  Azure::Core::Context ctx;
  auto response = m_privatePipeline->Send(request, ctx);

  auto const& headers = response->GetHeaders();
  auto findHeader = std::find_if(
      headers.begin(), headers.end(), [](std::pair<std::string const&, std::string const&> h) {
        return h.first == "x-recording-id";
      });
  m_testContext.RecordingId = findHeader->second;
  m_isPlaybackMode = true;
}

void TestProxyManager::SetStopPlaybackMode() {
  Azure::Core::Url stopPlaybackReq(m_proxy);
  stopPlaybackReq.AppendPath("playback");
  stopPlaybackReq.AppendPath("stop");
  Azure::Core::Http::Request request(Azure::Core::Http::HttpMethod::Post, stopPlaybackReq);
  request.SetHeader("x-recording-id", m_testContext.RecordingId);
  Azure::Core::Context ctx;
  m_privatePipeline->Send(request, ctx);

  m_isPlaybackMode = false;
  m_testContext.RecordingId.clear();
}

std::unique_ptr<Azure::Core::Http::Policies::HttpPolicy> TestProxyManager::GetRecordPolicy()
{
  return std::make_unique<Azure::Core::Test::RecordTestProxyPolicy>(this);
}

std::unique_ptr<Azure::Core::Http::Policies::HttpPolicy> TestProxyManager::GetPlaybackPolicy()
{
  return std::make_unique<Azure::Core::Test::PlaybackTestProxyPolicy>(this);
}