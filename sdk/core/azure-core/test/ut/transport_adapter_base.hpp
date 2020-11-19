// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

/**
 * @file
 * @brief The base class for the common bahavior of the transport adapter tests.
 *
 * @brief Any http trasport adapter can be used for this tests.
 *
 */

#include "gtest/gtest.h"
#include <azure/core/http/body_stream.hpp>
#include <azure/core/http/curl/curl.hpp>
#include <azure/core/http/http.hpp>
#include <azure/core/http/pipeline.hpp>
#include <azure/core/http/policy.hpp>

#include <azure/core/test/test_base.hpp>

#include <memory>
#include <vector>

using namespace Azure::Core::Test;

namespace Azure { namespace Core { namespace Test {

  class TransportAdapter
      : public Azure::Core::Test::TestBase,
        public testing::WithParamInterface<Azure::Core::Http::TransportPolicyOptions> {
  protected:
    std::unique_ptr<Azure::Core::Http::HttpPipeline> m_pipeline;

  public:
    // Befor each test, create pipeline
    virtual void SetUp() override
    {
      // init interceptor
      Azure::Core::Test::TestBase::SetUp();

      std::vector<std::unique_ptr<Azure::Core::Http::HttpPolicy>> policies;
      Azure::Core::Http::RetryOptions opt;
      opt.RetryDelay = std::chrono::milliseconds(10);

      // Retry policy will help to prevent server-occasionally-errors
      policies.push_back(std::make_unique<Azure::Core::Http::RetryPolicy>(opt));

      // Default transport policy and transport policy options
      Azure::Core::Http::TransportPolicyOptions transportOptions = GetParam();

      if (m_interceptor.IsPlaybackMode())
      {
        // Replace the transport adapter for the playback client
        transportOptions.Transport = std::move(m_interceptor.GetPlaybackClient());
      }
      else if (!m_interceptor.IsLiveMode())
      {
        // insert record policy
        policies.push_back(std::move(m_interceptor.GetRecordPolicy()));
      }

      // Record and Live mode uses whatever transport adapter the test is using for network
      policies.push_back(std::make_unique<Azure::Core::Http::TransportPolicy>(transportOptions));

      m_pipeline = std::make_unique<Azure::Core::Http::HttpPipeline>(policies);
    }

    static void CheckBodyFromBuffer(
        Azure::Core::Http::RawResponse& response,
        int64_t size,
        std::string expectedBody = std::string(""));

    static void CheckBodyFromStream(
        Azure::Core::Http::RawResponse& response,
        int64_t size,
        std::string expectedBody = std::string(""));

    static void checkResponseCode(
        Azure::Core::Http::HttpStatusCode code,
        Azure::Core::Http::HttpStatusCode expectedCode = Azure::Core::Http::HttpStatusCode::Ok);
  };

}}} // namespace Azure::Core::Test
