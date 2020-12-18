// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include "transport_adapter_base.hpp"

#include <azure/core/http/policy.hpp>
#include <azure/core/http/transport.hpp>

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
      Azure::Core::Http::TransportPolicyOptions options;
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
  INSTANTIATE_TEST_SUITE_P(
      Test,
      TransportAdapter,
      testing::Values(
#if defined(BUILD_TRANSPORT_WINHTTP_ADAPTER)
          /* WinHttp */
          GetTransportOptions("winHttp", std::make_shared<Azure::Core::Http::WinHttpTransport>())
#if defined(BUILD_CURL_HTTP_TRANSPORT_ADAPTER)
          /* WinHttp + LibCurl */
          ,
#endif
#endif
#if defined(BUILD_CURL_HTTP_TRANSPORT_ADAPTER)
          /* LibCurl */
          GetTransportOptions("libCurl", std::make_shared<Azure::Core::Http::CurlTransport>())
#endif
              ),
      GetSuffix);

}}} // namespace Azure::Core::Test
