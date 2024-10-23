// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include "transport_adapter_base_test.hpp"

#include <azure/core/http/policies/policy.hpp>
#include <azure/core/http/transport.hpp>

#if defined(BUILD_CURL_HTTP_TRANSPORT_ADAPTER)
#include "azure/core/http/curl_transport.hpp"
#endif

#if defined(BUILD_TRANSPORT_WINHTTP_ADAPTER)
#include "azure/core/http/win_http_transport.hpp"
#endif

#include <string>

using testing::ValuesIn;

namespace Azure { namespace Core { namespace Test {

  /**********************   Define the parameters for the base test and a suffix  ***************/
  namespace {
    // Produces a parameter for the transport adapters tests based in a suffix and an specific
    // adapter implementation
    static TransportAdaptersTestParameter GetTransportOptions(
        std::string suffix,
        std::shared_ptr<Azure::Core::Http::HttpTransport> adapter)
    {
      Azure::Core::Http::Policies::TransportOptions options;
      options.Transport = adapter;
      return TransportAdaptersTestParameter(std::move(suffix), options);
    }

    // When adding more than one parameter, this function should return a unique string.
    static std::string GetSuffix(const testing::TestParamInfo<TransportAdapter::ParamType>& info)
    {
      // Can't use empty spaces or underscores (_) as per google test documentation
      // https://github.com/google/googletest/blob/master/googletest/docs/advanced.md#specifying-names-for-value-parameterized-test-parameters
      return info.param.Suffix;
    }
  } // namespace

  /*********************** Transporter Adapter Tests ******************************/
  /*
   * MSVC does not support adding the pre-processor `#if` condition inside the MACRO parameters,
   * this is why we need to duplicate each case based on the transport adapters built.
   */
#if defined(BUILD_TRANSPORT_WINHTTP_ADAPTER) && defined(BUILD_CURL_HTTP_TRANSPORT_ADAPTER)
  /* WinHTTP + libcurl */
  INSTANTIATE_TEST_SUITE_P(
      Test,
      TransportAdapter,
      testing::Values(
          GetTransportOptions("winHttp", std::make_shared<Azure::Core::Http::WinHttpTransport>()),
          GetTransportOptions("libCurl", std::make_shared<Azure::Core::Http::CurlTransport>())),
      GetSuffix);

#elif defined(BUILD_TRANSPORT_WINHTTP_ADAPTER)
  /* WinHTTP */
  INSTANTIATE_TEST_SUITE_P(
      Test,
      TransportAdapter,
      testing::Values(
          GetTransportOptions("winHttp", std::make_shared<Azure::Core::Http::WinHttpTransport>())),
      GetSuffix);

#elif defined(BUILD_CURL_HTTP_TRANSPORT_ADAPTER)
  /* libcurl */
  INSTANTIATE_TEST_SUITE_P(
      Test,
      TransportAdapter,
      testing::Values(
          GetTransportOptions("libCurl", std::make_shared<Azure::Core::Http::CurlTransport>())),
      GetSuffix);
#else
  /* Custom adapter. Not adding tests */
#endif

  std::unique_ptr<Azure::Core::Http::_internal::HttpPipeline>
  TransportAdapter::CreateTlsClientAuthPipelineForTest()
  {
    if (GetParam().Suffix == "winHttp")
    {
      Azure::Core::Http::WinHttpTransportOptions options;
      options.TlsClientCertificate = nullptr;
      auto transport = std::make_shared<Azure::Core::Http::WinHttpTransport>(options);
      std::vector<std::unique_ptr<Azure::Core::Http::Policies::HttpPolicy>> retryPolicies;
      std::vector<std::unique_ptr<Azure::Core::Http::Policies::HttpPolicy>> policies;

      Azure::Core::_internal::ClientOptions op;
      op.Retry.RetryDelay = std::chrono::milliseconds(10);
      op.Transport.Transport = transport;
      auto pipeline = std::make_unique<Azure::Core::Http::_internal::HttpPipeline>(
          op, "TransportTest", "X.X", std::move(retryPolicies), std::move(policies));
      return pipeline;
    }
    return nullptr;
  }

}}} // namespace Azure::Core::Test
