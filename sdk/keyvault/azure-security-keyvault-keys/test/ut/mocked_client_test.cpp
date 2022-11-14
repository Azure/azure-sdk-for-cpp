//  Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include "gtest/gtest.h"

#include "mocked_transport_adapter_test.hpp"

#include <azure/core/internal/strings.hpp>
#include <azure/keyvault/keys.hpp>

#include <string>
using namespace Azure::Security::KeyVault::Keys;
using namespace Azure::Security::KeyVault::Keys::Test;

TEST_F(KeyVaultKeyClientMocked, keyvaultTelemetryId)
{
  std::string applicationId("ourApplicationId");
  m_clientOptions.Telemetry.ApplicationId = applicationId;
  m_client = std::make_unique<
      Azure::Security::KeyVault::Keys::Test::KeyClientWithNoAuthenticationPolicy>(
      "http://account.vault.azure.net", m_clientOptions);

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

TEST_F(KeyVaultKeyClientMocked, keyvaultTelemetryIdVersion)
{
  m_client = std::make_unique<
      Azure::Security::KeyVault::Keys::Test::KeyClientWithNoAuthenticationPolicy>(
      "url", m_clientOptions);

  std::string const expectedTelemetryVersionString(
      Azure::Security::KeyVault::Keys::_detail::PackageVersion::ToString());
  std::string telemetryStart("azsdk-cpp-keyvault-keys/");

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
            return Azure::Core::_internal::StringExtensions::LocaleInvariantCaseInsensitiveEqual(
                received, sent);
          },
          header.second.substr(telemetryStart.size(), expectedTelemetryVersionString.size()),
          expectedTelemetryVersionString);
      break;
    }
  }
  EXPECT_TRUE(foundHeader);
}

TEST_F(KeyVaultKeyClientMocked, CreateKeyRSA)
{
  std::string applicationId("CreateKeyRSA");
  m_clientOptions.Telemetry.ApplicationId = applicationId;
  m_client = std::make_unique<
      Azure::Security::KeyVault::Keys::Test::KeyClientWithNoAuthenticationPolicy>(
      "http://account.vault.azure.net", m_clientOptions);

  // The fake response from the mocked transport adapter is good for parsing a Key back
  auto response = m_client->CreateKey("name", KeyVaultKeyType::Rsa);

  EXPECT_EQ(response.Value.GetKeyType(), KeyVaultKeyType::Rsa);
}

TEST_F(KeyVaultKeyClientMocked, CreateKeyRSA2)
{
  std::string applicationId("CreateKeyRSA");
  m_clientOptions.Telemetry.ApplicationId = applicationId;
  m_client = std::make_unique<
      Azure::Security::KeyVault::Keys::Test::KeyClientWithNoAuthenticationPolicy>(
      "http://account.vault.azure.net", m_clientOptions);

  auto options = CreateRsaKeyOptions("name");
  // The fake response from the mocked transport adapter is good for parsing a Key back
  auto response = m_client->CreateRsaKey(options);

  EXPECT_EQ(response.Value.GetKeyType(), KeyVaultKeyType::Rsa);
}

// cspell: disable-next-line
TEST_F(KeyVaultKeyClientMocked, CreateKeyRSAHSM)
{
  // cspell: disable-next-line
  std::string applicationId("CreateKeyRSAHSM");
  m_clientOptions.Telemetry.ApplicationId = applicationId;
  m_client = std::make_unique<
      Azure::Security::KeyVault::Keys::Test::KeyClientWithNoAuthenticationPolicy>(
      "http://account.vault.azure.net", m_clientOptions);

  auto options = CreateRsaKeyOptions("name", true);
  // The fake response from the mocked transport adapter is good for parsing a Key back
  auto response = m_client->CreateRsaKey(options);

  EXPECT_EQ(response.Value.GetKeyType(), KeyVaultKeyType::RsaHsm);
}

TEST_F(KeyVaultKeyClientMocked, CreateKeyEC)
{
  std::string applicationId("CreateKeyEC");
  m_clientOptions.Telemetry.ApplicationId = applicationId;
  m_client = std::make_unique<
      Azure::Security::KeyVault::Keys::Test::KeyClientWithNoAuthenticationPolicy>(
      "http://account.vault.azure.net", m_clientOptions);

  auto options = CreateEcKeyOptions("name");
  // The fake response from the mocked transport adapter is good for parsing a Key back
  auto response = m_client->CreateEcKey(options);

  EXPECT_EQ(response.Value.GetKeyType(), KeyVaultKeyType::Ec);
}

// cspell: disable-next-line
TEST_F(KeyVaultKeyClientMocked, CreateKeyECHSM)
{
  // cspell: disable-next-line
  std::string applicationId("CreateKeyECHSM");
  m_clientOptions.Telemetry.ApplicationId = applicationId;
  m_client = std::make_unique<
      Azure::Security::KeyVault::Keys::Test::KeyClientWithNoAuthenticationPolicy>(
      "http://account.vault.azure.net", m_clientOptions);

  auto options = CreateEcKeyOptions("name", true);
  // The fake response from the mocked transport adapter is good for parsing a Key back
  auto response = m_client->CreateEcKey(options);

  EXPECT_EQ(response.Value.GetKeyType(), KeyVaultKeyType::EcHsm);
}

TEST_F(KeyVaultKeyClientMocked, CreateKeyOCT)
{
  std::string applicationId("CreateKeyOCT");
  m_clientOptions.Telemetry.ApplicationId = applicationId;
  m_client = std::make_unique<
      Azure::Security::KeyVault::Keys::Test::KeyClientWithNoAuthenticationPolicy>(
      "http://account.vault.azure.net", m_clientOptions);

  auto options = CreateOctKeyOptions("name");
  // The fake response from the mocked transport adapter is good for parsing a Key back
  auto response = m_client->CreateOctKey(options);

  EXPECT_EQ(response.Value.GetKeyType(), KeyVaultKeyType::Oct);
}

// cspell: disable-next-line
TEST_F(KeyVaultKeyClientMocked, CreateKeyOCTHSM)
{
  // cspell: disable-next-line
  std::string applicationId("CreateKeyOCTHSM");
  m_clientOptions.Telemetry.ApplicationId = applicationId;
  m_client = std::make_unique<
      Azure::Security::KeyVault::Keys::Test::KeyClientWithNoAuthenticationPolicy>(
      "http://account.vault.azure.net", m_clientOptions);

  auto options = CreateOctKeyOptions("name", true);
  // The fake response from the mocked transport adapter is good for parsing a Key back
  auto response = m_client->CreateOctKey(options);

  EXPECT_EQ(response.Value.GetKeyType(), KeyVaultKeyType::OctHsm);
}

TEST_F(KeyVaultKeyClientMocked, GetPropertiesOfKeys)
{
  std::string applicationId("CreateKey");
  m_clientOptions.Telemetry.ApplicationId = applicationId;
  m_client = std::make_unique<
      Azure::Security::KeyVault::Keys::Test::KeyClientWithNoAuthenticationPolicy>(
      "http://account.vault.azure.net", m_clientOptions);

  auto options = GetPropertiesOfKeysOptions();
  // The fake response from the mocked transport adapter is good for parsing a Key back
  auto response = m_client->GetPropertiesOfKeys();
  EXPECT_NE(response.RawResponse, nullptr);
}
