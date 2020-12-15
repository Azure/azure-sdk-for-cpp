// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include "transport_adapter_base.hpp"
#include <string>

#include <azure/core/http/winhttp/win_http_client.hpp>

using testing::ValuesIn;

namespace Azure { namespace Core { namespace Test {

  /**********************   Define the parameters for the base test and a suffix  ***************/
  namespace {
    static Azure::Core::Http::TransportPolicyOptions GetTransportOptions()
    {
      Azure::Core::Http::TransportPolicyOptions options;
      options.Transport = std::make_shared<Azure::Core::Http::WinHttpTransport>();
      return options;
    }

    // When adding more than one parameter, this function should return a unique string.
    // But since we are only using one parameter (the winhttp transport adapter) this is fine.
    static std::string GetSuffix(const testing::TestParamInfo<TransportAdapter::ParamType>& info)
    {
      // Can't use empty spaces or underscores (_) as per google test documentation
      // https://github.com/google/googletest/blob/master/googletest/docs/advanced.md#specifying-names-for-value-parameterized-test-parameters
      (void)(info);
      std::string suffix("winHttpImplementation");
      return suffix;
    }
  } // namespace

  /*********************** Base Transporter Adapter Tests ******************************/
  INSTANTIATE_TEST_SUITE_P(
      TransportAdapterWinHttpImpl,
      TransportAdapter,
      testing::Values(GetTransportOptions()),
      GetSuffix);

}}} // namespace Azure::Core::Test
