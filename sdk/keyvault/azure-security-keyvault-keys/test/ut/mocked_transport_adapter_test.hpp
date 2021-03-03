// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

/**
 * @file
 * @brief The base class to construct and init a Key Vault client.
 *
 */

#include <gtest/gtest.h>

#include <azure/core.hpp>
#include <azure/keyvault/key_vault.hpp>

#include <cstdio>

namespace Azure { namespace Security { namespace KeyVault { namespace Keys { namespace Test {

  namespace Details {
    // Return a simple key as response so keyvault can parse it to create the T response
    // Fake key from https://docs.microsoft.com/en-us/rest/api/keyvault/GetKey/GetKey#examples
    constexpr static const char FakeKey[]
        = "{  \"key\": {    \"kid\": "
          "\"https://myvault.vault.azure.net/keys/CreateSoftKeyTest/"
          "78deebed173b48e48f55abf87ed4cf71\",    \"kty\": \"RSA\",    \"key_ops\": [      "
          "\"encrypt\",      \"decrypt\",      \"sign\",      \"verify\",      \"wrapKey\",      "
          "\"unwrapKey\"    ]},  \"attributes\": {    \"enabled\": true,    "
          "\"created\": 1493942451,    \"updated\": 1493942451,    \"recoveryLevel\": "
          "\"Recoverable+Purgeable\"  },  \"tags\": {              \"purpose\" "
          ": "
          "\"unit test\", \"test name \" : \"CreateGetDeleteKeyTest\"}}";
  } // namespace Details

  // A transport adapter which only echo a request headers back as a response.
  class MockedTransportAdapter : public Azure::Core::Http::HttpTransport {
    std::unique_ptr<Azure::Core::Http::RawResponse> Send(
        Azure::Core::Context const& context,
        Azure::Core::Http::Request& request) override
    {
      (void)context;
      auto response = std::make_unique<Azure::Core::Http::RawResponse>(
          1, 1, Azure::Core::Http::HttpStatusCode::Ok, "Ok");

      // Copy headers
      for (auto header : request.GetHeaders())
      {
        response->AddHeader(header.first, header.second);
      }
      std::string bodyCount(Details::FakeKey);
      response->SetBodyStream(std::make_unique<Azure::IO::MemoryBodyStream>(
          reinterpret_cast<const uint8_t*>(Details::FakeKey), bodyCount.size()));
      return response;
    } // namespace Azure
  }; // namespace Test

  // A derived class with no credential and authentication
  class KeyClientWithNoAuthenticationPolicy : public Azure::Security::KeyVault::Keys::KeyClient {
  public:
    explicit KeyClientWithNoAuthenticationPolicy(
        std::string const& vaultUrl,
        KeyClientOptions options = KeyClientOptions())
        : KeyClient(vaultUrl, nullptr, options)
    {
      auto apiVersion = options.GetVersionString();

      // Base Pipeline
      std::vector<std::unique_ptr<Azure::Core::Http::HttpPolicy>> policies;
      policies.emplace_back(std::make_unique<Azure::Core::Http::TelemetryPolicy>(
          "KeyVault", apiVersion, options.TelemetryPolicyOptions));
      policies.emplace_back(std::make_unique<Azure::Core::Http::RequestIdPolicy>());
      policies.emplace_back(std::make_unique<Azure::Core::Http::RetryPolicy>(options.RetryOptions));
      policies.emplace_back(std::make_unique<Azure::Core::Http::LogPolicy>(options.LogOptions));
      policies.emplace_back(
          std::make_unique<Azure::Core::Http::TransportPolicy>(options.TransportPolicyOptions));
      Azure::Core::Http::Url url(vaultUrl);

      m_pipeline = std::make_unique<Azure::Security::KeyVault::Common::Internal::KeyVaultPipeline>(
          url, apiVersion, std::move(policies));
    }
  };

  class MockedTransportAdapterTest : public ::testing::Test {
  protected:
    std::unique_ptr<KeyClientWithNoAuthenticationPolicy> m_client;
    Azure::Security::KeyVault::Keys::KeyClientOptions m_clientOptions;

    // Create
    virtual void SetUp() override
    {
      m_clientOptions.TransportPolicyOptions.Transport = std::make_shared<MockedTransportAdapter>();
    }
  };
}}}}} // namespace Azure::Security::KeyVault::Keys::Test
