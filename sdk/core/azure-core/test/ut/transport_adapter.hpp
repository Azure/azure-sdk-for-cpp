// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include "gtest/gtest.h"
#include <http/body_stream.hpp>
#include <http/curl/curl.hpp>
#include <http/http.hpp>
#include <http/pipeline.hpp>

#include <memory>
#include <vector>

namespace Azure { namespace Core { namespace Test {

  class TransportAdapter : public ::testing::Test {
  protected:
    static Azure::Core::Http::HttpPipeline pipeline;
    static std::vector<std::unique_ptr<Azure::Core::Http::HttpPolicy>> policies;
    static Azure::Core::Context context;

    static void CheckBodyStreamLength(
        Azure::Core::Http::Response& response,
        int64_t size,
        std::string expectedBody = std::string(""));
  };

}}} // namespace Azure::Core::Test
