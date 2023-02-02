// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include "azure/keyvault/secrets/keyvault_deleted_secret.hpp"
#include "azure/keyvault/secrets/keyvault_secret.hpp"
#include <azure/core/http/http.hpp>
#include <azure/core/http/policies/policy.hpp>
#include <gtest/gtest.h>
#include <string>

using namespace Azure::Security::KeyVault::Secrets;
using namespace Azure::Core::Http::_internal;

namespace Azure { namespace Security { namespace KeyVault { namespace Secrets { namespace _test {
  struct Helpers
  {

    static Azure::Core::Http::RawResponse GetPartialResponse()
    {
      auto response
          = Azure::Core::Http::RawResponse(1, 1, Azure::Core::Http::HttpStatusCode::Ok, "OK");

      constexpr static const uint8_t responseBody[] = R"json({
        "value": "my_secret_value",
        "id": "https://myvault.vault.azure.net/secrets/my_secret_name/4387e9f3d6e14c459867679a90fd0f79",
        "managed":true,
        "attributes": {
          "enabled": true,
          "created": 1493938410,
          "updated": 1493938410,
          "recoveryLevel": "Recoverable+Purgeable"
         }
    }
)json";

      response.SetHeader(HttpShared::ContentType, "application/json");
      response.SetHeader(HttpShared::MsRequestId, "1");
      response.SetHeader(HttpShared::MsClientRequestId, "2");
      response.SetBody(std::vector<uint8_t>(responseBody, responseBody + sizeof(responseBody)));
      response.SetBodyStream(std::make_unique<Azure::Core::IO::MemoryBodyStream>(
          responseBody, sizeof(responseBody) - 1));

      return response;
    }

    static Azure::Core::Http::RawResponse GetFullResponse()
    {
      auto response
          = Azure::Core::Http::RawResponse(1, 1, Azure::Core::Http::HttpStatusCode::Ok, "OK");

      constexpr static const uint8_t responseBody[] = R"json({
        "value": "my_secret_value",
        "id": "https://myvault.vault.azure.net/secrets/my_secret_name/4387e9f3d6e14c459867679a90fd0f79",
        "contentType" : "ct",
        "kid": "kid",
        "managed": true,
        "attributes": {
          "enabled": true,
          "created": 1493938410,
          "updated": 1493938410,
          "recoveryLevel": "Recoverable+Purgeable"
         }
    }
)json";

      response.SetHeader(HttpShared::ContentType, "application/json");
      response.SetHeader(HttpShared::MsRequestId, "1");
      response.SetHeader(HttpShared::MsClientRequestId, "2");
      response.SetBody(std::vector<uint8_t>(responseBody, responseBody + sizeof(responseBody)));
      response.SetBodyStream(std::make_unique<Azure::Core::IO::MemoryBodyStream>(
          responseBody, sizeof(responseBody) - 1));

      return response;
    }

    static Azure::Core::Http::RawResponse GetDeletedFullResponse()

    {
      auto response
          = Azure::Core::Http::RawResponse(1, 1, Azure::Core::Http::HttpStatusCode::Ok, "OK");

      constexpr static const uint8_t responseBody[] = R"json({
        "recoveryId": "https://myvault.vault.azure.net/deletedsecrets/GetDeletedSecretTest",
        "deletedDate": 1493938433,
        "scheduledPurgeDate": 1501714433,
        "managed": true,
        "id": "https://myvault.vault.azure.net/secrets/my_secret_name/4387e9f3d6e14c459867679a90fd0f79",
        "attributes": {
          "enabled": true,
          "created": 1493938433,
          "updated": 1493938433,
          "recoveryLevel": "Recoverable+Purgeable"
        }
})json";

      response.SetHeader(HttpShared::ContentType, "application/json");
      response.SetHeader(HttpShared::MsRequestId, "1");
      response.SetHeader(HttpShared::MsClientRequestId, "2");
      response.SetBody(std::vector<uint8_t>(responseBody, responseBody + sizeof(responseBody)));
      response.SetBodyStream(std::make_unique<Azure::Core::IO::MemoryBodyStream>(
          responseBody, sizeof(responseBody) - 1));

      return response;
    }

    static void RunPartialExpect(KeyVaultSecret& secret, bool expectValue = true)
    {
      if (expectValue)
      {
        EXPECT_EQ(secret.Value.Value(), "my_secret_value");
      }

      EXPECT_EQ(secret.Name, "my_secret_name");
      EXPECT_EQ(secret.Properties.VaultUrl, "https://myvault.vault.azure.net");
      EXPECT_EQ(secret.Properties.Version, "4387e9f3d6e14c459867679a90fd0f79");
      EXPECT_EQ(secret.Properties.Id, secret.Id);
      EXPECT_EQ(
          secret.Id,
          "https://myvault.vault.azure.net/secrets/my_secret_name/"
          "4387e9f3d6e14c459867679a90fd0f79");
      EXPECT_EQ(secret.Properties.Managed, true);
      EXPECT_EQ(secret.Properties.KeyId.HasValue(), false);
      EXPECT_EQ(secret.Properties.UpdatedOn.HasValue(), true);
      EXPECT_EQ(secret.Properties.CreatedOn.HasValue(), true);
    }

    static void RunFullExpect(KeyVaultSecret& secret, bool expectValue = true)
    {
      if (expectValue)
      {
        EXPECT_EQ(secret.Value.Value(), "my_secret_value");
        EXPECT_EQ(secret.Properties.ContentType.Value(), "ct");
        EXPECT_EQ(secret.Properties.KeyId.Value(), "kid");
      }

      EXPECT_EQ(secret.Name, "my_secret_name");
      EXPECT_EQ(secret.Properties.VaultUrl, "https://myvault.vault.azure.net");
      EXPECT_EQ(secret.Properties.Version, "4387e9f3d6e14c459867679a90fd0f79");
      EXPECT_EQ(secret.Properties.Id, secret.Id);
      EXPECT_EQ(
          secret.Id,
          "https://myvault.vault.azure.net/secrets/my_secret_name/"
          "4387e9f3d6e14c459867679a90fd0f79");
      EXPECT_EQ(secret.Properties.Enabled.Value(), true);
      EXPECT_EQ(secret.Properties.Managed, true);
      EXPECT_EQ(secret.Properties.UpdatedOn.HasValue(), true);
      EXPECT_EQ(secret.Properties.CreatedOn.HasValue(), true);
    }

    static void RunDeletedExtras(DeletedSecret& secret)
    {
      EXPECT_EQ(
          secret.RecoveryId, "https://myvault.vault.azure.net/deletedsecrets/GetDeletedSecretTest");
      EXPECT_EQ(secret.ScheduledPurgeDate.Value().ToString(), "2017-08-02T22:53:53Z");
      EXPECT_EQ(secret.DeletedOn.Value().ToString(), "2017-05-04T22:53:53Z");
    }
  };
}}}}} // namespace Azure::Security::KeyVault::Secrets::_test
