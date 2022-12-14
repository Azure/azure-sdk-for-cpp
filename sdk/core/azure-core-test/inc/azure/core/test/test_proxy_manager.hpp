//  Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

/**
 * Test proxy mamager class
 */

#pragma once

#include <azure/core/credentials/credentials.hpp>
#include <azure/core/http/http.hpp>
#include <azure/core/http/policies/policy.hpp>
#include <memory>
#include <string>

#include "azure/core/test/network_models.hpp"
#include "azure/core/test/test_context_manager.hpp"
#include "azure/core/test/test_proxy_policy.hpp"
#if defined(BUILD_CURL_HTTP_TRANSPORT_ADAPTER)
#include <azure/core/http/curl_transport.hpp>
#endif
#if defined(BUILD_TRANSPORT_WINHTTP_ADAPTER)
#include <azure/core/http/win_http_transport.hpp>
#endif
#include <azure/core/http/policies/policy.hpp>
#include <azure/core/internal/http/pipeline.hpp>

using namespace Azure::Core::Http::_internal;

namespace Azure { namespace Core { namespace Test {

  class TestNonExpiringCredential final : public Core::Credentials::TokenCredential {
  public:
    Core::Credentials::AccessToken GetToken(
        Core::Credentials::TokenRequestContext const& tokenRequestContext,
        Core::Context const& context) const override
    {
      Core::Credentials::AccessToken accessToken;
      accessToken.Token = "magicToken";
      accessToken.ExpiresOn = DateTime::max();

      if (context.IsCancelled() || tokenRequestContext.Scopes.size() == 0)
      {
        accessToken.ExpiresOn = DateTime::min();
      }

      return accessToken;
    }
  };

  class TestProxyManager {
  private:
    // Using a reference because the context lives in the test_base class and we don't want to make
    // a copy.
    Azure::Core::Test::TestContextManager& m_testContext;
    const std::string m_proxy = "https://localhost:5001";
    bool m_isInsecureEnabled = true;
    TestMode m_currentMode = TestMode::LIVE;
    std::unique_ptr<Azure::Core::Http::_internal::HttpPipeline> m_privatePipeline;

  public:
    /**
     * @brief Configures the transport to ignore certificate validation
     *
     */
    void ConfigureInsecureConnection(Azure::Core::_internal::ClientOptions& clientOptions);

    /**
     * @brief Enables to init TestProxyManager with empty values.
     *
     */
    TestProxyManager(Azure::Core::Test::TestContextManager& testContext)
        : m_testContext(testContext)
    {
      Azure::Core::_internal::ClientOptions clientOp;
      clientOp.Retry.MaxRetries = 0;
      ConfigureInsecureConnection(clientOp);
      std::vector<std::unique_ptr<Azure::Core::Http::Policies::HttpPolicy>> policiesOp;
      std::vector<std::unique_ptr<Azure::Core::Http::Policies::HttpPolicy>> policiesRe;
      Azure::Core::Http::_internal::HttpPipeline pipeline(
          clientOp, "PerfFw", "na", std::move(policiesRe), std::move(policiesOp));
      m_privatePipeline = std::make_unique<Azure::Core::Http::_internal::HttpPipeline>(pipeline);
      SetProxySanitizer();
    }

    /**
     * Are we in RECORD mode
     *
     * @return bool indicating RECORD mode
     */
    bool IsRecordMode() { return m_currentMode == TestMode::RECORD; }

    /**
     * Are we in PLAYBACK mode
     *
     * @return bool indicating PLAYBACK mode
     */
    bool IsPlaybackMode() { return m_currentMode == TestMode::PLAYBACK; }

    /**
     * Gets the proxy https url
     *
     * @return string containing the https url of the proxy (e.g. "https://localhost:xyz")
     */
    std::string GetTestProxy() { return m_proxy; };

    /**
     * Gets a ref to the test context
     *
     * @return Test context ref
     */
    Azure::Core::Test::TestContextManager& GetTestContext() { return m_testContext; }

    /**
     * Gets HTTP pipeline policy that records network calls and its data is managed by the
     * TestProxy.
     *
     * @return HttpPipelinePolicy to record network calls.
     */
    std::unique_ptr<Azure::Core::Http::Policies::HttpPolicy> GetTestProxyPolicy();

    /**
     * @brief Read from environment and parse the a test mode.
     *
     * @remark If the AZURE_TEST_MODE variable is not found, default test mode is LIVE mode.
     *
     * @return TestMode
     */
    static TestMode GetTestMode();

    /**
     * @brief Get a non-expiring token credential. This is a test utility for use in playback
     * scenarios where the token is not relevant.
     *
     * @return std::shared_ptr<Core::Credentials::TokenCredential>
     */
    std::shared_ptr<Core::Credentials::TokenCredential> GetTestCredential()
    {
      return std::make_shared<TestNonExpiringCredential>();
    }

    /**
     * Sets proxy to stop RECORD test and save the recording file.
     *
     */
    void SetStopRecordMode();

    /**
     * Sets proxy to stop PLAYBACK test.
     *
     */
    void SetStopPlaybackMode();

    /**
     * Gets the test recording ID
     *
     * @returns recording ID
     */
    std::string GetRecordingId() { return m_testContext.RecordingId; }

    void StartPlaybackRecord(TestMode testMode);
    void StopPlaybackRecord(TestMode testMode);

  private:
    std::string PrepareRequestBody();
    void SetProxySanitizer();
    bool CheckSanitizers();
  };

}}} // namespace Azure::Core::Test
