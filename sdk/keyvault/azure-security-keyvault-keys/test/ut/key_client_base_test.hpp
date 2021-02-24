// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

/**
 * @file
 * @brief The base class to construct and init a Key Vault client.
 *
 */

#include <gtest/gtest.h>

#include <azure/core/context.hpp>
#include <azure/identity/client_secret_credential.hpp>
#include <azure/keyvault/key_vault.hpp>

#include <cstdio>

namespace Azure { namespace Security { namespace KeyVault { namespace Keys { namespace Test {

  class KeyVaultClientTest : public ::testing::Test {
  protected:
    std::shared_ptr<Azure::Identity::ClientSecretCredential> m_credential;
    std::string m_keyVaultUrl;
    std::string m_keyVaultHsmUrl;
    std::unique_ptr<Azure::Security::KeyVault::Keys::KeyClient> m_client;

    // Create
    virtual void SetUp() override
    {
      std::string tenantId = std::getenv("AZURE_TENANT_ID");
      std::string clientId = std::getenv("AZURE_CLIENT_ID");
      std::string secretId = std::getenv("AZURE_CLIENT_SECRET");
      m_credential
          = std::make_shared<Azure::Identity::ClientSecretCredential>(tenantId, clientId, secretId);

      m_keyVaultUrl = std::getenv("AZURE_KEYVAULT_URL");
    }

  public:
    template <class T>
    static inline void CheckValidResponse(
        Azure::Core::Response<T>& response,
        Azure::Core::Http::HttpStatusCode expectedCode = Azure::Core::Http::HttpStatusCode::Ok)
    {
      auto rawResponse = response.ExtractRawResponse();
      EXPECT_EQ(rawResponse->GetStatusCode(), expectedCode);
    }
  };

}}}}} // namespace Azure::Security::KeyVault::Keys::Test
