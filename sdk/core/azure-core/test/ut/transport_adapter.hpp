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

#include <memory>
#include <vector>

namespace Azure { namespace Core { namespace Test {

  class TransportAdapter : public ::testing::Test {
  protected:
    static Azure::Core::Http::HttpPipeline pipeline;
    static std::vector<std::unique_ptr<Azure::Core::Http::HttpPolicy>> policies;
    static Azure::Core::Context context;

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
