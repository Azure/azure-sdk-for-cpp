// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#if defined(_MSC_VER)
#define _CRT_SECURE_NO_WARNINGS
#endif

#include "gtest/gtest.h"

#include "mocked_transport_adapter_test.hpp"

#include <azure/core/internal/strings.hpp>
#include <azure/keyvault/keyvault_keys.hpp>

#include <string>
using namespace Azure::Security::KeyVault::Keys;
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

  // The response is an echo of the sent headers. Let's find the telemetry ID
  auto foundHeader = false;
  for (auto& header : response.RawResponse->GetHeaders())
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

TEST_F(MockedTransportAdapterTest, CreateKeyRSA)
{
  std::string applicationId("CreateKeyRSA");
  m_clientOptions.Telemetry.ApplicationId = applicationId;
  m_client = std::make_unique<
      Azure::Security::KeyVault::Keys::Test::KeyClientWithNoAuthenticationPolicy>(
      "url", m_clientOptions);

  // The fake response from the mocked transport adapter is good for parsing a Key back
  auto response = m_client->CreateKey("name", KeyVaultKeyType::Rsa);

  EXPECT_EQ(response.Value.GetKeyType(), KeyVaultKeyType::Rsa);
}

TEST_F(MockedTransportAdapterTest, CreateKeyRSA2)
{
  std::string applicationId("CreateKeyRSA");
  m_clientOptions.Telemetry.ApplicationId = applicationId;
  m_client = std::make_unique<
      Azure::Security::KeyVault::Keys::Test::KeyClientWithNoAuthenticationPolicy>(
      "url", m_clientOptions);

  auto options = CreateRsaKeyOptions("name");
  // The fake response from the mocked transport adapter is good for parsing a Key back
  auto response = m_client->CreateRsaKey(options);

  EXPECT_EQ(response.Value.GetKeyType(), KeyVaultKeyType::Rsa);
}

TEST_F(MockedTransportAdapterTest, CreateKeyRSAHSM)
{
  std::string applicationId("CreateKeyRSAHSM");
  m_clientOptions.Telemetry.ApplicationId = applicationId;
  m_client = std::make_unique<
      Azure::Security::KeyVault::Keys::Test::KeyClientWithNoAuthenticationPolicy>(
      "url", m_clientOptions);

  auto options = CreateRsaKeyOptions("name", true);
  // The fake response from the mocked transport adapter is good for parsing a Key back
  auto response = m_client->CreateRsaKey(options);

  EXPECT_EQ(response.Value.GetKeyType(), KeyVaultKeyType::RsaHsm);
}

TEST_F(MockedTransportAdapterTest, CreateKeyEC)
{
  std::string applicationId("CreateKeyEC");
  m_clientOptions.Telemetry.ApplicationId = applicationId;
  m_client = std::make_unique<
      Azure::Security::KeyVault::Keys::Test::KeyClientWithNoAuthenticationPolicy>(
      "url", m_clientOptions);

  auto options = CreateEcKeyOptions("name");
  // The fake response from the mocked transport adapter is good for parsing a Key back
  auto response = m_client->CreateEcKey(options);

  EXPECT_EQ(response.Value.GetKeyType(), KeyVaultKeyType::Ec);
}

TEST_F(MockedTransportAdapterTest, CreateKeyECHSM)
{
  std::string applicationId("CreateKeyECHSM");
  m_clientOptions.Telemetry.ApplicationId = applicationId;
  m_client = std::make_unique<
      Azure::Security::KeyVault::Keys::Test::KeyClientWithNoAuthenticationPolicy>(
      "url", m_clientOptions);

  auto options = CreateEcKeyOptions("name", true);
  // The fake response from the mocked transport adapter is good for parsing a Key back
  auto response = m_client->CreateEcKey(options);

  EXPECT_EQ(response.Value.GetKeyType(), KeyVaultKeyType::EcHsm);
}

TEST_F(MockedTransportAdapterTest, CreateKeyOCT)
{
  std::string applicationId("CreateKeyOCT");
  m_clientOptions.Telemetry.ApplicationId = applicationId;
  m_client = std::make_unique<
      Azure::Security::KeyVault::Keys::Test::KeyClientWithNoAuthenticationPolicy>(
      "url", m_clientOptions);

  auto options = CreateOctKeyOptions("name");
  // The fake response from the mocked transport adapter is good for parsing a Key back
  auto response = m_client->CreateOctKey(options);

  EXPECT_EQ(response.Value.GetKeyType(), KeyVaultKeyType::Oct);
}

TEST_F(MockedTransportAdapterTest, CreateKeyOCTHSM)
{
  std::string applicationId("CreateKeyOCTHSM");
  m_clientOptions.Telemetry.ApplicationId = applicationId;
  m_client = std::make_unique<
      Azure::Security::KeyVault::Keys::Test::KeyClientWithNoAuthenticationPolicy>(
      "url", m_clientOptions);

  auto options = CreateOctKeyOptions("name", true);
  // The fake response from the mocked transport adapter is good for parsing a Key back
  auto response = m_client->CreateOctKey(options);

  EXPECT_EQ(response.Value.GetKeyType(), KeyVaultKeyType::OctHsm);
}

TEST_F(MockedTransportAdapterTest, GetPropertiesOfKeys)
{
  std::string applicationId("CreateKey");
  m_clientOptions.Telemetry.ApplicationId = applicationId;
  m_client = std::make_unique<
      Azure::Security::KeyVault::Keys::Test::KeyClientWithNoAuthenticationPolicy>(
      "url", m_clientOptions);

  auto options = GetPropertiesOfKeysOptions();
  // The fake response from the mocked transport adapter is good for parsing a Key back
  auto response = m_client->GetPropertiesOfKeys();
  EXPECT_NE(response.RawResponse, nullptr);
}
