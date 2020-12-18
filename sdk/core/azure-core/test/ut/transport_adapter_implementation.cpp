// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include "transport_adapter_base.hpp"
#include <azure/core/context.hpp>
#include <azure/core/http/policy.hpp>
#include <azure/core/response.hpp>

#include <iostream>
#include <string>
#include <thread>

#include <http/curl/curl_connection_pool_private.hpp>
#include <http/curl/curl_connection_private.hpp>

using testing::ValuesIn;

namespace Azure { namespace Core { namespace Test {

  /**********************   Define the parameters for the base test and a suffix  ***************/
  namespace {
    static TransportAdaptersTestParameter GetTransportOptions(
        std::string prefix,
        std::shared_ptr<Azure::Core::Http::CurlTransport> adapter)
    {
      Azure::Core::Http::TransportPolicyOptions options;
      options.Transport = adapter;
      return TransportAdaptersTestParameter(std::move(prefix), options);
    }

    // When adding more than one parameter, this function should return a unique string.
    // But since we are only using one parameter (the libcurl transport adapter) this is fine.
    static std::string GetSuffix(const testing::TestParamInfo<TransportAdapter::ParamType>& info)
    {
      // Can't use empty spaces or underscores (_) as per google test documentation
      // https://github.com/google/googletest/blob/master/googletest/docs/advanced.md#specifying-names-for-value-parameterized-test-parameters
      return info.param.Prefix;
    }
  } // namespace

  /*********************** Transporter Adapter Tests ******************************/
  INSTANTIATE_TEST_SUITE_P(
      TransportAdapterCurlImpl,
      TransportAdapter,
      testing::Values(
#if defined(AZ_PLATFORM_WINDOWS)
          GetTransportOptions(
              "winTransportAdapter",
              std::make_shared<Azure::Core::Http::WinHttpTransport>()),
#endif
          GetTransportOptions(
              "curlTransportAdapter",
              std::make_shared<Azure::Core::Http::CurlTransport>())),
      GetSuffix);

}}} // namespace Azure::Core::Test
