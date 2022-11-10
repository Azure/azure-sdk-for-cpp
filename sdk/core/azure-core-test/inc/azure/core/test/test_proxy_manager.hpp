#pragma once

#include <azure/core/credentials/credentials.hpp>
#include <azure/core/http/http.hpp>
#include <azure/core/http/policies/policy.hpp>
#include <memory>
#include <string>

#include "azure/core/test/network_models.hpp"
#include "azure/core/test/record_test_proxy_policy.hpp"
#include "azure/core/test/test_context_manager.hpp"
#if defined(BUILD_CURL_HTTP_TRANSPORT_ADAPTER)
#include <azure/core/http/curl_transport.hpp>
#endif
#if defined(BUILD_TRANSPORT_WINHTTP_ADAPTER)
#include <azure/core/http/win_http_transport.hpp>
#endif
#include <azure/core/http/policies/policy.hpp>
#include <azure/core/internal/http/pipeline.hpp>

// Used by recordPolicy and playback transport adapter.
#if !defined(RECORDING_BODY_STREAM_SENTINEL)
#define RECORDING_BODY_STREAM_SENTINEL "__bodyStream__"
#endif

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
    void ConfigureInsecureConnection(Azure::Core::_internal::ClientOptions& clientOptions);
    bool m_isRecordMode = false;

  public:
    /**
     * @brief Enables to init an interceptor with empty values.
     *
     */
    TestProxyManager(Azure::Core::Test::TestContextManager& testContext)
        : m_testContext(testContext)
    {
    }

    bool IsRecordMode() { return m_isRecordMode; }

    std::string GetTestProxy() { return m_proxy; };
    Azure::Core::Test::TestContextManager& GetTestContext() { return m_testContext; }
    /**
     * Gets HTTP pipeline policy that records network calls and its data is managed by the
     * TestProxy.
     *
     * @return HttpPipelinePolicy to record network calls.
     */
    std::unique_ptr<Azure::Core::Http::Policies::HttpPolicy> GetRecordPolicy();
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

    void SetStartRecordMode();
    void SetStopRecordMode();
    bool SetStartPlaybackMode(){};
    bool SetStopPlaybackMode(){};
    std::string GetRecordingId() { return m_testContext.RecordingId; }
  };

}}} // namespace Azure::Core::Test