// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include "azure/core/test/test_proxy_manager.hpp"

#include <azure/core/internal/environment.hpp>
#include <azure/core/internal/json/json.hpp>
#include <azure/core/internal/strings.hpp>

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

namespace {
const char* g_accountRegex = "https://(?<account>[a-zA-Z0-9\\-]+)\\.";
}

bool TestProxyManager::CheckSanitizers()
{
  Azure::Core::Url checkRequest(m_proxy);
  checkRequest.AppendPath("Info");
  checkRequest.AppendPath("Active");

  Azure::Core::Http::Request request(Azure::Core::Http::HttpMethod::Get, checkRequest);
  Azure::Core::Context ctx;
  auto response = m_privatePipeline->Send(request, ctx);

  auto rawResponse = response->GetBody();
  std::string stringBody(rawResponse.begin(), rawResponse.end());
  if (stringBody.find(g_accountRegex) == std::string::npos)
  {
    return false;
  }
  return true;
}

void TestProxyManager::SetProxySanitizer()
{
  if (CheckSanitizers())
  {
    return;
  }
  // we have 3 types of sanitizer,
  // see
  // https://github.com/Azure/azure-sdk-tools/blob/main/tools/test-proxy/Azure.Sdk.Tools.TestProxy/README.md#a-note-about-where-sanitizers-apply
  enum class SanitizerType
  {
    Uri,
    Header,
    Body,
    General,
  };
  auto addSanitizer = [&](SanitizerType type,
                          const std::string& regex,
                          const std::string& groupName,
                          const std::string& headerName = std::string()) {
    const std::map<SanitizerType, std::string> abstractionIdentifierValues = {
        {SanitizerType::Uri, "UriRegexSanitizer"},
        {SanitizerType::Header, "HeaderRegexSanitizer"},
        {SanitizerType::Body, "BodyRegexSanitizer"},
        {SanitizerType::General, "GeneralRegexSanitizer"},
    };

    Azure::Core::Url sanitizerRequest(m_proxy);
    sanitizerRequest.AppendPath("Admin");
    sanitizerRequest.AppendPath("AddSanitizer");

    auto jsonRoot = Json::_internal::json::object();
    jsonRoot["value"] = "REDACTED";
    jsonRoot["regex"] = regex;
    jsonRoot["groupForReplace"] = groupName;
    if (!headerName.empty())
    {
      jsonRoot["key"] = headerName;
    }
    auto jsonString = jsonRoot.dump();

    Azure::Core::IO::MemoryBodyStream payloadStream(
        reinterpret_cast<const uint8_t*>(jsonString.data()), jsonString.size());
    Azure::Core::Http::Request request(
        Azure::Core::Http::HttpMethod::Post, sanitizerRequest, &payloadStream);
    request.SetHeader("x-abstraction-identifier", abstractionIdentifierValues.at(type));
    Azure::Core::Context ctx;
    auto response = m_privatePipeline->Send(request, ctx);
    (void)response;
  };

  addSanitizer(SanitizerType::General, g_accountRegex, "account");
  addSanitizer(SanitizerType::Body, "client_secret=(?<clientsecret>[^&]+)", "clientsecret");
  const std::string storageSasSignatureRegex = "\\?.*sig=(?<sassig>[a-zA-Z0-9\\%\\/+=]+)";
  addSanitizer(SanitizerType::Uri, storageSasSignatureRegex, "sassig");
  addSanitizer(SanitizerType::Header, storageSasSignatureRegex, "sassig", "x-ms-copy-source");
  addSanitizer(SanitizerType::Header, storageSasSignatureRegex, "sassig", "x-ms-rename-source");
  addSanitizer(SanitizerType::Header, "(?<auth>.+)", "auth", "x-ms-copy-source-authorization");
  addSanitizer(SanitizerType::Header, "(?<cookie>.+)", "cookie", "Cookie");
  addSanitizer(SanitizerType::Header, "(?<cookie>.+)", "cookie", "Set-Cookie");
  const std::string storageUserDelegationKeyRegex
      = "\\u003CValue\\u003E(?<userdelegationkey>[a-zA-Z0-9\\/=+]+).*\\u003C\\/"
        "UserDelegationKey\\u003E";
  addSanitizer(SanitizerType::Body, storageUserDelegationKeyRegex, "userdelegationkey");

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
        "Cookie",
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
        "x-ms-immutability-policy-until-date",
    };
    const std::vector<std::string> ignoreQueryParameters = {
        "st",
        "se",
        "sig",
        "sv",
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
        reinterpret_cast<const uint8_t*>(matcherBody.data()), matcherBody.size());
    Azure::Core::Http::Request request(
        Azure::Core::Http::HttpMethod::Post, matcherRequest, &payloadStream);
    request.SetHeader("x-abstraction-identifier", "CustomDefaultMatcher");
    Azure::Core::Context ctx;
    auto response = m_privatePipeline->Send(request, ctx);
  }
}
