// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include "gtest/gtest.h"
#include <azure/core/http/body_stream.hpp>
#include <azure/core/http/curl/curl.hpp>
#include <azure/core/http/http.hpp>
#include <azure/core/http/pipeline.hpp>
#include <azure/core/test/test_base.hpp>

#include <memory>
#include <vector>

using namespace Azure::Core::Test;

namespace Azure { namespace Core { namespace Test {

  class TransportAdapterTest : public TestBase {
  protected:
    static Azure::Core::Context context;

    // Each test creates its own pipeline.
    // This enables diferent pipeline configuration depending on the Interceptor Manager.
    std::unique_ptr<Azure::Core::Http::HttpPipeline> m_pipeline;

    /**
     * @brief Run before each test
     *
     */
    void SetUp() override
    {
      // init interceptor
      Azure::Core::Test::TestBase::SetUp();

      // init pipeline
      std::vector<std::unique_ptr<Azure::Core::Http::HttpPolicy>> policies;
      std::shared_ptr<Azure::Core::Http::HttpTransport> transport
          = std::make_shared<Azure::Core::Http::CurlTransport>();
      Azure::Core::Http::RetryOptions opt;
      opt.RetryDelay = std::chrono::milliseconds(10);

      // Retry policy will help to prevent server-occasionally-errors
      policies.push_back(std::make_unique<Azure::Core::Http::RetryPolicy>(opt));

      if (m_interceptor.IsPlaybackMode())
      {
        policies.push_back(std::make_unique<Azure::Core::Http::TransportPolicy>(
            std::move(m_interceptor.GetPlaybackClient())));
      }
      else if (!m_interceptor.IsLiveMode())
      {
        // This means Record mode.  No Playback and no Live mode
        policies.push_back(std::move(m_interceptor.GetRecordPolicy()));
      }

      // Record and Live mode uses a real transport adapter
      policies.push_back(
          std::make_unique<Azure::Core::Http::TransportPolicy>(std::move(transport)));

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
