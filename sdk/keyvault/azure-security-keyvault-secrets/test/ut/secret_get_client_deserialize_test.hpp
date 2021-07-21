// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT


#include <azure/core/http/http.hpp>
#include <azure/core/http/policies/policy.hpp>
#include <azure/keyvault/secrets/keyvault_secret.hpp>

#include <gtest/gtest.h>
#include <string>

using namespace Azure::Security::KeyVault::Secrets;
using namespace Azure::Core::Http::_internal;

namespace {
Azure::Core::Http::RawResponse getPartialResponse()
{
  auto response = Azure::Core::Http::RawResponse(1, 1, Azure::Core::Http::HttpStatusCode::Ok, "OK");

  constexpr static const uint8_t responseBody[] = R"json({
        "value": "mysecretvalue",
        "id": "https://myvault.vault.azure.net/secrets/mysecretname/4387e9f3d6e14c459867679a90fd0f79",
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
  response.SetBodyStream(
      std::make_unique<Azure::Core::IO::MemoryBodyStream>(responseBody, sizeof(responseBody) - 1));

  return response;
}

Azure::Core::Http::RawResponse getFullResponse()

{
  auto response = Azure::Core::Http::RawResponse(1, 1, Azure::Core::Http::HttpStatusCode::Ok, "OK");

  constexpr static const uint8_t responseBody[] = R"json({
        "value": "mysecretvalue",
        "id": "https://myvault.vault.azure.net/secrets/mysecretname/4387e9f3d6e14c459867679a90fd0f79",
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
  response.SetBodyStream(
      std::make_unique<Azure::Core::IO::MemoryBodyStream>(responseBody, sizeof(responseBody) - 1));

  return response;
}

void runPartialExpect(KeyVaultSecret& secret)
{
  EXPECT_EQ(secret.Value, "mysecretvalue");
  EXPECT_EQ(
      secret.Id,
      "https://myvault.vault.azure.net/secrets/mysecretname/4387e9f3d6e14c459867679a90fd0f79");
  EXPECT_EQ(secret.KeyId.HasValue(), false);
  EXPECT_EQ(secret.Attributes.Enabled.Value(), true);
  EXPECT_EQ(secret.Managed, false);
  EXPECT_EQ(secret.Attributes.UpdatedOn.HasValue(), true);
  EXPECT_EQ(secret.Attributes.CreatedOn.HasValue(), true);
}

void runFullExpect(KeyVaultSecret& secret)
{
  EXPECT_EQ(secret.Value, "mysecretvalue");
  EXPECT_EQ(
      secret.Id,
      "https://myvault.vault.azure.net/secrets/mysecretname/4387e9f3d6e14c459867679a90fd0f79");
  EXPECT_EQ(secret.Attributes.Enabled.Value(), true);
  EXPECT_EQ(secret.Managed, true);
  EXPECT_EQ(secret.ContentType.Value(), "ct");
  EXPECT_EQ(secret.KeyId.Value(), "kid");
  EXPECT_EQ(secret.Attributes.UpdatedOn.HasValue(), true);
  EXPECT_EQ(secret.Attributes.CreatedOn.HasValue(), true);
}

} // namespace