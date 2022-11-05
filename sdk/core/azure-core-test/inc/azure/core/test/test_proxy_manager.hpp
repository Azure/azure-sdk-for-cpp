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
	class TestProxyManager {
  private:
    // Using a reference because the context lives in the test_base class and we don't want to make
    // a copy.
    Azure::Core::Test::TestContextManager& m_testContext;
    Azure::Core::Url m_testProxyUrl;
    Azure::Core::Url m_testProxyStartRecordingUrl;
    Azure::Core::Url m_testProxyStopRecordingUrl;

    std::string m_requestId;
    const std::string header_requestId = "x-recording-id";
    const std::string header_file_location = "x-recording-file";
    const std::string header_recording_base_uri = "x-recording-upstream-base-uri";
    const std::string op_start = "start";
    const std::string op_stop = "stop";
    const std::string direction_playback = "playback";
    const std::string direction_record = "record";

  public:
    /**
     * @brief Enables to init a test proxy manager with empty values.
     *
     */
    TestProxyManager(Azure::Core::Test::TestContextManager& testContext)
        : m_testContext(testContext)
    {
      m_testProxyUrl.SetHost("http://localHost::9000/");
      m_testProxyStartRecordingUrl.SetHost(m_testProxyUrl.GetAbsoluteUrl());
      m_testProxyStartRecordingUrl.AppendPath(direction_record);
      m_testProxyStartRecordingUrl.AppendPath(op_start);
      m_testProxyStopRecordingUrl.SetHost(m_testProxyUrl.GetAbsoluteUrl());
      m_testProxyStopRecordingUrl.AppendPath(direction_record);
      m_testProxyStopRecordingUrl.AppendPath(op_stop);
    }
  };
}}} // namespace Azure::Core::Test