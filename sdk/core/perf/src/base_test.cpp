// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include "azure/perf/base_test.hpp"

#if defined(BUILD_CURL_HTTP_TRANSPORT_ADAPTER)
#include <azure/core/http/curl_transport.hpp>
#endif
#if defined(BUILD_TRANSPORT_WINHTTP_ADAPTER)
#include <azure/core/http/win_http_transport.hpp>
#endif
#include <azure/core/http/policies/policy.hpp>
#include <azure/core/internal/http/pipeline.hpp>
#include <azure/identity/client_secret_credential.hpp>
#include <azure/identity/default_azure_credential.hpp>

#include <functional>
#include <string>
#include <vector>

using namespace Azure::Core::Http;
using namespace Azure::Core::Http::Policies;
using namespace Azure::Core;
namespace {

class ProxyPolicy final : public HttpPolicy {
private:
  Azure::Perf::BaseTest* m_testContext;

public:
  ProxyPolicy(Azure::Perf::BaseTest* proxyManager) : m_testContext(proxyManager) {}

  // copy
  ProxyPolicy(ProxyPolicy const& other) : ProxyPolicy{other.m_testContext} {}

  // move
  ProxyPolicy(ProxyPolicy&& other) noexcept : m_testContext{other.m_testContext} {}

  std::unique_ptr<RawResponse> Send(
      Request& request,
      NextHttpPolicy nextPolicy,
      Context const& context) const override
  {
    std::string const recordId(m_testContext->m_recordId);
    if (recordId.empty())
    {
      return nextPolicy.Send(request, context);
    }

    // Use a new request to redirect
    auto redirectRequest = Azure::Core::Http::Request(
        request.GetMethod(), Azure::Core::Url(m_testContext->m_proxy), request.GetBodyStream());
    if (!request.ShouldBufferResponse())
    {
      // This is a download with keep connection open. Let's switch the request
      redirectRequest = Azure::Core::Http::Request(
          request.GetMethod(), Azure::Core::Url(m_testContext->m_proxy), false);
    }
    redirectRequest.GetUrl().SetPath(request.GetUrl().GetPath());

    // Copy all headers
    for (auto& header : request.GetHeaders())
    {
      redirectRequest.SetHeader(header.first, header.second);
    }
    // QP
    for (auto const& qp : request.GetUrl().GetQueryParameters())
    {
      redirectRequest.GetUrl().AppendQueryParameter(qp.first, qp.second);
    }
    // Set x-recording-upstream-base-uri
    {
      auto const& url = request.GetUrl();
      auto const port = url.GetPort();
      auto const host
          = url.GetScheme() + "://" + url.GetHost() + (port != 0 ? ":" + std::to_string(port) : "");
      redirectRequest.SetHeader("x-recording-upstream-base-uri", host);
    }
    // Set recording-id
    redirectRequest.SetHeader("x-recording-id", recordId);
    redirectRequest.SetHeader("x-recording-remove", "false");

    // Using recordId, find out MODE
    if (m_testContext->m_isPlayBackMode)
    {
      // PLAYBACK mode
      redirectRequest.SetHeader("x-recording-mode", "playback");
    }
    else
    {
      // RECORDING mode
      redirectRequest.SetHeader("x-recording-mode", "record");
    }

    return nextPolicy.Send(redirectRequest, context);
  }

  std::unique_ptr<HttpPolicy> Clone() const override
  {
    return std::make_unique<ProxyPolicy>(*this);
  }
};

} // namespace

namespace Azure { namespace Perf {

  void BaseTest::ConfigureInsecureConnection(Azure::Core::_internal::ClientOptions& clientOptions)
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

  void BaseTest::ConfigureClientOptions(Azure::Core::_internal::ClientOptions& clientOptions)
  {
    if (!m_proxy.empty())
    {
      clientOptions.PerRetryPolicies.push_back(std::make_unique<ProxyPolicy>(this));
    }
    ConfigureInsecureConnection(clientOptions);
  }

  void BaseTest::PostSetUp()
  {
    if (!m_proxy.empty())
    {
      Azure::Core::_internal::ClientOptions clientOp;
      clientOp.Retry.MaxRetries = 0;
      ConfigureInsecureConnection(clientOp);
      std::vector<std::unique_ptr<Azure::Core::Http::Policies::HttpPolicy>> policiesOp;
      std::vector<std::unique_ptr<Azure::Core::Http::Policies::HttpPolicy>> policiesRe;
      Azure::Core::Http::_internal::HttpPipeline pipeline(
          clientOp, "PerfFw", "na", std::move(policiesRe), std::move(policiesOp));
      Azure::Core::Context ctx;

      //  Make one call to Run() before starting recording, to avoid capturing one-time setup
      //  like authorization requests.
      this->Run(ctx);

      // Send start-record call
      {
        Azure::Core::Url startRecordReq(m_proxy);
        startRecordReq.AppendPath("record");
        startRecordReq.AppendPath("start");
        Azure::Core::Http::Request request(Azure::Core::Http::HttpMethod::Post, startRecordReq);
        auto response = pipeline.Send(request, ctx);

        auto const& headers = response->GetHeaders();
        auto findHeader = std::find_if(
            headers.begin(),
            headers.end(),
            [](std::pair<std::string const&, std::string const&> h) {
              return h.first == "x-recording-id";
            });
        m_recordId = findHeader->second;
      }

      // Record one call to re-use response on all test runs
      this->Run(ctx);

      // Stop recording
      {
        Azure::Core::Url stopRecordReq(m_proxy);
        stopRecordReq.AppendPath("record");
        stopRecordReq.AppendPath("stop");
        Azure::Core::Http::Request request(Azure::Core::Http::HttpMethod::Post, stopRecordReq);
        request.SetHeader("x-recording-id", m_recordId);
        pipeline.Send(request, ctx);
      }

      // Start playback
      {
        Azure::Core::Url startPlayback(m_proxy);
        startPlayback.AppendPath("playback");
        startPlayback.AppendPath("start");
        Azure::Core::Http::Request request(Azure::Core::Http::HttpMethod::Post, startPlayback);
        request.SetHeader("x-recording-id", m_recordId);
        auto response = pipeline.Send(request, ctx);
        auto const& headers = response->GetHeaders();
        auto findHeader = std::find_if(
            headers.begin(),
            headers.end(),
            [](std::pair<std::string const&, std::string const&> h) {
              return h.first == "x-recording-id";
            });
        m_recordId = findHeader->second;
        m_isPlayBackMode = true;
      }
    }
  }

  void BaseTest::PreCleanUp()
  {
    if (!m_recordId.empty())
    {
      Azure::Core::_internal::ClientOptions clientOp;
      clientOp.Retry.MaxRetries = 0;
      ConfigureInsecureConnection(clientOp);
      std::vector<std::unique_ptr<Azure::Core::Http::Policies::HttpPolicy>> policiesOp;
      std::vector<std::unique_ptr<Azure::Core::Http::Policies::HttpPolicy>> policiesRe;
      Azure::Core::Http::_internal::HttpPipeline pipeline(
          clientOp, "PerfFw", "na", std::move(policiesRe), std::move(policiesOp));
      Azure::Core::Context ctx;

      // Stop playback
      {
        Azure::Core::Url stopPlaybackReq(m_proxy);
        stopPlaybackReq.AppendPath("playback");
        stopPlaybackReq.AppendPath("stop");
        Azure::Core::Http::Request request(Azure::Core::Http::HttpMethod::Post, stopPlaybackReq);
        request.SetHeader("x-recording-id", m_recordId);
        request.SetHeader("x-purge-inmemory-recording", "true"); // cspell:disable-line

        pipeline.Send(request, ctx);

        m_recordId.clear();
        m_isPlayBackMode = false;
      }
    }
  }

  class TestNonExpiringCredential final : public Core::Credentials::TokenCredential {
  public:
    TestNonExpiringCredential() : TokenCredential("TestNonExpiringCredential") {}

    Core::Credentials::AccessToken GetToken(
        Core::Credentials::TokenRequestContext const& tokenRequestContext,
        Core::Context const& context) const override
    {
      Core::Credentials::AccessToken accessToken;
      accessToken.Token = "magicToken";
      accessToken.ExpiresOn = (DateTime::max)();

      if (context.IsCancelled() || tokenRequestContext.Scopes.size() == 0)
      {
        accessToken.ExpiresOn = (DateTime::min)();
      }

      return accessToken;
    }
  };

  std::shared_ptr<Azure::Core::Credentials::TokenCredential> BaseTest::GetTestCredential()
  {
    if (m_testCredential)
    {
      return m_testCredential;
    }
    if (m_isPlayBackMode)
    {
      // Playback mode uses:
      //  - never-expiring test credential to never require a token
      return std::make_shared<TestNonExpiringCredential>();
    }
    else
    {
      std::string clientSecret;
      try
      {
        clientSecret = GetEnv("AZURE_CLIENT_SECRET");
      }
      catch (std::runtime_error&)
      {
      }
      catch (...)
      {
        throw;
      }
      if (clientSecret.empty())
      {
        m_testCredential = std::make_shared<Azure::Identity::DefaultAzureCredential>();
      }
      else
      {
        m_testCredential = std::make_shared<Azure::Identity::ClientSecretCredential>(
            GetEnv("AZURE_TENANT_ID"), GetEnv("AZURE_CLIENT_ID"), clientSecret);
      }

      return m_testCredential;
    }
  }

  /**
   * @brief Utility function used by tests to retrieve env vars
   *
   * @param name Environment variable name to retrieve.
   *
   * @return The value of the environment variable retrieved.
   *
   * @note If AZURE_TENANT_ID, AZURE_CLIENT_ID, or AZURE_CLIENT_SECRET are not available in the
   * environment, the AZURE_SERVICE_DIRECTORY environment variable is used to set those values
   * with the values emitted by the New-TestResources.ps1 script.
   *
   * @note The Azure CI pipeline upper cases all environment variables defined in the pipeline.
   * Since some operating systems have case sensitive environment variables, on debug builds,
   * this function ensures that the environment variable being retrieved is all upper case.
   *
   */
  std::string BaseTest::GetEnv(std::string const& name)
  {
#if !defined(NDEBUG)
    // The azure CI pipeline uppercases all EnvVar values from ci.yml files.
    // That means that any mixed case strings will not be found when run from the CI
    // pipeline. Check to make sure that the developer only passed in an upper case environment
    // variable.
    {
      if (name != Azure::Core::_internal::StringExtensions::ToUpper(name))
      {
        throw std::runtime_error("All Azure SDK environment variables must be all upper case.");
      }
    }
#endif
    auto ret = Azure::Core::_internal::Environment::GetVariable(name.c_str());
    if (ret.empty())
    {
      static const char azurePrefix[] = "AZURE_";
      if (!m_isPlayBackMode && name.find(azurePrefix) == 0)
      {
        std::string serviceDirectory
            = Azure::Core::_internal::Environment::GetVariable("AZURE_SERVICE_DIRECTORY");
        if (serviceDirectory.empty())
        {
          throw std::runtime_error(
              "Could not find a value for " + name
              + " and AZURE_SERVICE_DIRECTORY was not defined. Define either " + name
              + " or AZURE_SERVICE_DIRECTORY to resolve.");
        }
        // Upper case the serviceName environment variable because all ci.yml environment
        // variables are upper cased.
        std::string serviceDirectoryEnvVar
            = Azure::Core::_internal::StringExtensions::ToUpper(serviceDirectory);
        serviceDirectoryEnvVar += name.substr(sizeof(azurePrefix) - 2);
        ret = Azure::Core::_internal::Environment::GetVariable(serviceDirectoryEnvVar.c_str());
        if (!ret.empty())
        {
          return ret;
        }
      }
      throw std::runtime_error("Missing required environment variable: " + name);
    }

    return ret;
  }

}} // namespace Azure::Perf
