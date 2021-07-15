// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT
#if defined(_MSC_VER)
#define _CRT_SECURE_NO_WARNINGS
#endif

#if defined(BUILD_CURL_HTTP_TRANSPORT_ADAPTER)
#include <curl/curl.h>
#endif


#include <azure/keyvault/secrets/secret_client.hpp>
#include <azure/identity.hpp>

using namespace Azure::Security::KeyVault::Secrets;

int main()
{
  auto tenantId = "72f988bf-86f1-41af-91ab-2d7cd011db47"; // std::getenv("AZURE_TENANT_ID");
  auto clientId = "e75e5cbc-4dcc-406b-a6d8-fd2ca7a4a900"; // std::getenv("AZURE_CLIENT_ID");
  auto clientSecret = "i5-X7l-HPErHhQ_0s0hyrHB793M-y.yi4w";// std::getenv("AZURE_CLIENT_SECRET");
  auto credential
      = std::make_shared<Azure::Identity::ClientSecretCredential>(tenantId, clientId, clientSecret);

  SecretClient secretClient(std::getenv("AZURE_KEYVAULT_URL"), credential);

  auto response = secretClient.GetSecret("testSecret");

  return 0;
}
