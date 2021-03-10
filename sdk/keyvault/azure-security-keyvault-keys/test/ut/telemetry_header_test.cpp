// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#if defined(_MSC_VER)
#define _CRT_SECURE_NO_WARNINGS
#endif

#include "gtest/gtest.h"

#include "mocked_transport_adapter_test.hpp"

#include <azure/core/internal/strings.hpp>
#include <azure/keyvault/key_vault.hpp>

#include <string>

using namespace Azure::Security::KeyVault::Keys::Test;

TEST_F(MockedTransportAdapterTest, keyvaultTelemetryId)
{
  std::string applicationId("ourApplicationId");
  m_clientOptions.Telemetry.ApplicationId = applicationId;
  m_client = std::make_unique<
      Azure::Security::KeyVault::Keys::Test::KeyClientWithNoAuthenticationPolicy>(
      "url", m_clientOptions);

  // The fake response from the mocked transport adapter is good for parsing a Key back
  auto response = m_client->GetKey("name");

  // The response is an echo of the sent headers. Let's find the telemetry id
  auto foundHeader = false;
  for (auto& header : response.GetRawResponse().GetHeaders())
  {
    if (Azure::Core::_internal::StringExtensions::LocaleInvariantCaseInsensitiveEqual(
            header.first, "User-Agent"))
    {
      foundHeader = true;
      EXPECT_PRED2(
          [](std::string const& received, std::string const& sent) {
            auto telemetryInfoWithNoOSAndDate = received.substr(0, sent.size());
            return Azure::Core::_internal::StringExtensions::LocaleInvariantCaseInsensitiveEqual(
                telemetryInfoWithNoOSAndDate, sent);
          },
          header.second,
          applicationId);
      break;
    }
  }
  EXPECT_TRUE(foundHeader);
}
