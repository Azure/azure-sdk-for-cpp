// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

/**
 * @brief Keep the state of the playback-record-live tests.
 *
 * @remark The interceptor is a singleton that is init during the test configuration.
 * Depending on the test mode, the interceptor will handle the recorder data.
 *
 * - If test mode is LIVE, the Interceptor will not affect the test behavior.
 * - If test mode is RECORD, the Interceptor will init the `record data` to be written after
 * capturing each request going out to the network and also recording the server response for that
 * request.
 * - If test mode is PLAYBACK, the interceptor will load the `record data` and use it to answer HTTP
 * client request without sending the request to the network.
 *
 * @remark The interceptor handle the `recorded data, provides the HTTP transport adapter and the
 * record policy. However, adding the policy and adapter to a pipeline is done by the user.
 */

#pragma once

#include <azure/core/credentials/credentials.hpp>
#include <azure/core/http/http.hpp>
#include <azure/core/http/policies/policy.hpp>
#include <memory>
#include <string>

#include "azure/core/test/network_models.hpp"
#include "azure/core/test/playback_http_client.hpp"
#include "azure/core/test/record_network_call_policy.hpp"
#include "azure/core/test/test_context_manager.hpp"

// Used by recordPolicy and playback transport adapter.
#if !defined(RECORDING_BODY_STREAM_SENTINEL)
#define RECORDING_BODY_STREAM_SENTINEL "__bodyStream__"
#endif
namespace Azure { namespace Core { namespace Test {

  /**
   * @brief TestNonExpiringCredential Credential authenticates with the Azure services using a
   * Tenant ID, Client ID and a client secret.
   *
   */
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

  /**
   * @brief A class that keeps track of network calls by either reading the data from an existing
   * test session record or recording the network calls in memory.
   *
   */
  class InterceptorManager {
  private:
    Azure::Core::Test::RecordedData m_recordedData;
    // Using a reference because the context lives in the test_base class and we don't want to make
    // a copy.
    Azure::Core::Test::TestContextManager& m_testContext;

  public:
    /**
     * @brief Enables to init an interceptor with empty values.
     *
     */
    InterceptorManager(Azure::Core::Test::TestContextManager& testContext)
        : m_testContext(testContext)
    {
    }

    /**
     * Gets the recorded data reference that InterceptorManager is keeping track of.
     *
     * @return The recorded data reference managed by InterceptorManager.
     */
    Azure::Core::Test::RecordedData& GetRecordedData() { return m_recordedData; }

    /**
     * Gets HTTP pipeline policy that records network calls and its data is managed by the
     * InterceptorManager.
     *
     * @return HttpPipelinePolicy to record network calls.
     */
    std::unique_ptr<Azure::Core::Http::Policies::HttpPolicy> GetRecordPolicy()
    {
      return std::make_unique<Azure::Core::Test::RecordNetworkCallPolicy>(this);
    }

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
     * Gets a new HTTP transport adapter that plays back test session records managed by the
     * InterceptorManager.
     *
     * @return An HTTP transport adapter that plays back network calls from its recorded data.
     */
    std::unique_ptr<Azure::Core::Http::HttpTransport> GetPlaybackTransport()
    {
      return std::make_unique<Azure::Core::Test::PlaybackClient>(this);
    }

    /**
     * @brief Get the Test Context object.
     *
     * @return Azure::Core::Test::TestContextManager const&
     */
    Azure::Core::Test::TestContextManager const& GetTestContext() const { return m_testContext; }

    /**
     * @brief Read from environment and parse the a test mode.
     *
     * @remark If the AZURE_TEST_MODE variable is not found, default test mode is LIVE mode.
     *
     * @return TestMode
     */
    static TestMode GetTestMode();

    /**
     * @brief This function is expected to be called by the playback transport adapter.
     *
     * @remark The name of the test is known and set when the test is actually started. That's why
     * the recorded data can't be loaded until the test is already running (Can't load on SetUp).
     *
     */
    void LoadTestData();

    /**
     * @brief Removes sensitive info from a request Url.
     *
     * @param url The request Url.
     * @return Azure::Core::Url
     */
    Azure::Core::Url RedactUrl(Azure::Core::Url const& url);
  };

}}} // namespace Azure::Core::Test