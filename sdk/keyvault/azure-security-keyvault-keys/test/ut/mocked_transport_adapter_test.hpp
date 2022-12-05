// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

/**
 * @file
 * @brief The base class to construct and init a Key Vault client.
 *
 */

#include <gtest/gtest.h>

#include "./../../src/private/key_serializers.hpp"
#include "./../../src/private/keyvault_protocol.hpp"
#include "./../../src/private/package_version.hpp"

#include <azure/core.hpp>
#include <azure/core/internal/http/pipeline.hpp>
#include <azure/keyvault/keys.hpp>

#include <cstdio>
#include <string>

namespace Azure { namespace Security { namespace KeyVault { namespace Keys { namespace Test {

  namespace _detail {
    // Return a simple key as response so keyvault can parse it to create the T response
    // Fake key from https://docs.microsoft.com/rest/api/keyvault/GetKey/GetKey#examples
    static const char FakeKey[]
        = "{  \"key\": {    \"kid\": "
          "\"https://myvault.vault.azure.net/keys/CreateSoftKeyTest/"
          "78deebed173b48e48f55abf87ed4cf71\",    \"kty\": \"%s\",    \"key_ops\": [      "
          "\"encrypt\",      \"decrypt\",      \"sign\",      \"verify\",      \"wrapKey\",      "
          "\"unwrapKey\"    ]},  \"attributes\": {    \"enabled\": true,    "
          "\"created\": 1493942451,    \"updated\": 1493942451,    \"recoveryLevel\": "
          "\"Recoverable+Purgeable\"  },  \"tags\": {              \"purpose\" "
          ": "
          "\"unit test\", \"test name \" : \"CreateGetDeleteKeyTest\"}}";
  } // namespace _detail

  // A transport adapter which only echo a request headers back as a response.
  class MockedTransportAdapter final : public Azure::Core::Http::HttpTransport {
    std::unique_ptr<Azure::Core::Http::RawResponse> Send(
        Azure::Core::Http::Request& request,
        Azure::Core::Context const& context) override
    {
      (void)context;
      auto response = std::make_unique<Azure::Core::Http::RawResponse>(
          1, 1, Azure::Core::Http::HttpStatusCode::Ok, "Ok");

      // Copy headers
      for (auto header : request.GetHeaders())
      {
        response->SetHeader(header.first, header.second);
      }
      auto updatedFakeKey = UpdateFakeKey(_detail::FakeKey, request.GetHeaders()["user-agent"]);
      std::string bodyCount(updatedFakeKey);
      response->SetBodyStream(std::make_unique<Azure::Core::IO::MemoryBodyStream>(
          reinterpret_cast<const uint8_t*>(updatedFakeKey), bodyCount.size()));
      return response;
    }

    const char* UpdateFakeKey(const char fakeKey[], std::string header)
    {
      char* result;
      std::string keyType = "RSA";
      // cspell: disable-next-line
      if (header.find("CreateKeyRSAHSM") != std::string::npos)
      {
        keyType = "RSA-HSM";
      }
      // cspell: disable-next-line
      else if (header.find("CreateKeyECHSM") != std::string::npos)
      {
        keyType = "EC-HSM";
      }
      // cspell: disable-next-line
      else if (header.find("CreateKeyOCTHSM") != std::string::npos)
      {
        keyType = "oct-HSM";
      }
      else if (header.find("CreateKeyRSA") != std::string::npos)
      {
        keyType = "RSA";
      }
      else if (header.find("CreateKeyEC") != std::string::npos)
      {
        keyType = "EC";
      }
      else if (header.find("CreateKeyOCT") != std::string::npos)
      {
        keyType = "oct";
      }

      result = new char[std::string(fakeKey).size() + keyType.size()];

#if defined(_MSC_VER)
#pragma warning(push)
#pragma warning(disable : 4996)
#endif
      std::sprintf(result, fakeKey, keyType.c_str());
#if defined(_MSC_VER)
#pragma warning(pop)
#endif

      return result;
    }
  };

  // A derived class with no credential and authentication
  class KeyClientWithNoAuthenticationPolicy final
      : public Azure::Security::KeyVault::Keys::KeyClient {
  public:
    explicit KeyClientWithNoAuthenticationPolicy(
        std::string const& vaultUrl,
        KeyClientOptions const& options = KeyClientOptions())
        : KeyClient(vaultUrl, nullptr, options)
    {
      std::vector<std::unique_ptr<Azure::Core::Http::Policies::HttpPolicy>> perCallpolicies;
      std::vector<std::unique_ptr<Azure::Core::Http::Policies::HttpPolicy>> perRetrypolicies;
      m_pipeline = std::make_unique<Azure::Core::Http::_internal::HttpPipeline>(
          options,
          "keyvault-keys",
          Azure::Security::KeyVault::Keys::_detail::PackageVersion::ToString(),
          std::move(perRetrypolicies),
          std::move(perCallpolicies));
    }
  };

  class KeyVaultKeyClientMocked : public ::testing::Test {
  protected:
    std::unique_ptr<KeyClientWithNoAuthenticationPolicy> m_client;
    Azure::Security::KeyVault::Keys::KeyClientOptions m_clientOptions;

    // Create
    virtual void SetUp() override
    {
      m_clientOptions.Transport.Transport = std::make_shared<MockedTransportAdapter>();
    }
  };
}}}}} // namespace Azure::Security::KeyVault::Keys::Test