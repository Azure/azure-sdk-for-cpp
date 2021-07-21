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

#include <azure/core/http/policy.hpp>
#include <memory>
#include <string>

#include "azure/core/test/network_models.hpp"
#include "azure/core/test/playback_http_client.hpp"
#include "azure/core/test/record_network_call_policy.hpp"
#include "azure/core/test/test_context_manager.hpp"

namespace Azure { namespace Core { namespace Test {

  /**
   * @brief A class that keeps track of network calls by either reading the data from an existing
   * test session record or recording the network calls in memory.
   *
   */
  class InterceptorManager {
  private:
    std::string m_testSession;
    TestMode m_testMode;
    Azure::Core::Test::RecordedData m_recordedData;

  public:
    /**
     * @brief Enables to init an interceptor with empty values.
     *
     */
    InterceptorManager(){};

    explicit InterceptorManager(Azure::Core::Test::TestContextManager testContext)
        : m_testSession(testContext.GetTestName()), m_testMode(testContext.GetTestMode())
    {
    }

    /**
     * Gets whether this InterceptorManager is in playback mode.
     *
     * @return `true` if the InterceptorManager is in playback mode and `false` otherwise.
     */
    bool IsPlaybackMode() { return m_testMode == TestMode::PLAYBACK; }

    /**
     * Gets whether this InterceptorManager is in live mode.
     *
     * @return `true` if the InterceptorManager is in live mode and `false` otherwise.
     */
    bool IsLiveMode() { return m_testMode == TestMode::LIVE; }

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
    std::unique_ptr<Azure::Core::Http::HttpPolicy> GetRecordPolicy()
    {
      return std::make_unique<Azure::Core::Test::RecordNetworkCallPolicy>(m_recordedData);
    }

    /**
     * Gets a new HTTP client that plays back test session records managed by {@link
     * InterceptorManager}.
     *
     * @return An HTTP client that plays back network calls from its recorded data.
     */
    std::unique_ptr<Azure::Core::Http::HttpTransport> GetPlaybackClient()
    {
      return std::make_unique<Azure::Core::Test::PlaybackClient>(m_recordedData);
    }

    /**
     * @brief Read from environment and parse the a test mode.
     *
     * @remark If the AZURE_TEST_MODE variable is not found, default test mode is LIVE mode.
     *
     * @return TestMode
     */
    static TestMode GetTestMode();
  };

}}} // namespace Azure::Core::Test
