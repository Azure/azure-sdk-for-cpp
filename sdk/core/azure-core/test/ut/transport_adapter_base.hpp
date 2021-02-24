// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

/**
 * @file
 * @brief The base class for the common bahavior of the transport adapter tests.
 *
 * @brief Any http trasport adapter can be used for this tests.
 *
 */

#include <azure/core/io/body_stream.hpp>
#include <azure/core/http/http.hpp>
#include <azure/core/internal/http/pipeline.hpp>
#include <gtest/gtest.h>

#include <memory>
#include <vector>

namespace Azure { namespace Core { namespace Test {

  struct TransportAdaptersTestParameter
  {
    std::string Suffix;
    Azure::Core::Http::TransportPolicyOptions TransportAdapter;

    TransportAdaptersTestParameter(
        std::string suffix,
        Azure::Core::Http::TransportPolicyOptions options)
        : Suffix(std::move(suffix)), TransportAdapter(std::move(options))
    {
    }
  };

  class TransportAdapter : public testing::TestWithParam<TransportAdaptersTestParameter> {
  protected:
    std::unique_ptr<Azure::Core::Internal::Http::HttpPipeline> m_pipeline;

    // Befor each test, create pipeline
    virtual void SetUp() override
    {
      std::vector<std::unique_ptr<Azure::Core::Http::HttpPolicy>> policies;
      Azure::Core::Http::RetryOptions opt;
      opt.RetryDelay = std::chrono::milliseconds(10);

      // Retry policy will help to prevent server-occasionally-errors
      policies.push_back(std::make_unique<Azure::Core::Http::RetryPolicy>(opt));
      // Will get transport policy options from test param
      // auto param = GetParam();
      policies.push_back(
          std::make_unique<Azure::Core::Http::TransportPolicy>(GetParam().TransportAdapter));

      m_pipeline = std::make_unique<Azure::Core::Internal::Http::HttpPipeline>(policies);
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
