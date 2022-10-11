// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

/**
 * @file
 * @brief The base class for the common bahavior of the transport adapter tests.
 *
 * @brief Any HTTP transport adapter can be used for this tests.
 *
 */

#include <azure/core/http/http.hpp>
#include <azure/core/internal/http/pipeline.hpp>
#include <azure/core/io/body_stream.hpp>
#include <gtest/gtest.h>

#include <memory>
#include <vector>

namespace Azure { namespace Core { namespace Test {

  namespace _detail {
    constexpr static const char AzureSdkHttpbinServerSchema[] = "https";
    constexpr static const char AzureSdkHttpbinServer[] = "azuresdkforcpp.azurewebsites.net";
  } // namespace _detail

  struct AzureSdkHttpbinServer final
  {
    inline static std::string Get()
    {
      return std::string(_detail::AzureSdkHttpbinServerSchema) + "://"
          + std::string(_detail::AzureSdkHttpbinServer) + "/get";
    }
    inline static std::string Headers()
    {
      return std::string(_detail::AzureSdkHttpbinServerSchema) + "://"
          + std::string(_detail::AzureSdkHttpbinServer) + "/headers";
    }
    inline static std::string WithPort()
    {
      return std::string(_detail::AzureSdkHttpbinServerSchema) + "://"
          + std::string(_detail::AzureSdkHttpbinServer) + ":443/get";
    }
    inline static std::string Put()
    {
      return std::string(_detail::AzureSdkHttpbinServerSchema) + "://"
          + std::string(_detail::AzureSdkHttpbinServer) + "/put";
    }
    inline static std::string Delete()
    {
      return std::string(_detail::AzureSdkHttpbinServerSchema) + "://"
          + std::string(_detail::AzureSdkHttpbinServer) + "/delete";
    }
    inline static std::string Patch()
    {
      return std::string(_detail::AzureSdkHttpbinServerSchema) + "://"
          + std::string(_detail::AzureSdkHttpbinServer) + "/patch";
    }
    inline static std::string Delay()
    {
      return std::string(_detail::AzureSdkHttpbinServerSchema) + "://"
          + std::string(_detail::AzureSdkHttpbinServer) + "/delay";
    }
    inline static std::string Host() { return std::string(_detail::AzureSdkHttpbinServer); }
    inline static std::string Schema() { return std::string(_detail::AzureSdkHttpbinServerSchema); }
  };

  struct TransportAdaptersTestParameter final
  {
    std::string Suffix;
    Azure::Core::Http::Policies::TransportOptions TransportAdapter;

    TransportAdaptersTestParameter(
        std::string suffix,
        Azure::Core::Http::Policies::TransportOptions options)
        : Suffix(std::move(suffix)), TransportAdapter(std::move(options))
    {
    }
  };

  class TransportAdapter : public testing::TestWithParam<TransportAdaptersTestParameter> {
  protected:
    std::unique_ptr<Azure::Core::Http::_internal::HttpPipeline> m_pipeline;

    // Befor each test, create pipeline
    virtual void SetUp() override
    {
      std::vector<std::unique_ptr<Azure::Core::Http::Policies::HttpPolicy>> retryPolicies;
      std::vector<std::unique_ptr<Azure::Core::Http::Policies::HttpPolicy>> policies;

      Azure::Core::_internal::ClientOptions op;
      op.Retry.RetryDelay = std::chrono::milliseconds(10);
      op.Transport = GetParam().TransportAdapter;
      m_pipeline = std::make_unique<Azure::Core::Http::_internal::HttpPipeline>(
          op, "TransportTest", "X.X", std::move(retryPolicies), std::move(policies));
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
