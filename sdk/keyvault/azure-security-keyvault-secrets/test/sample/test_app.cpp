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
  auto tenantId = std::getenv("AZURE_TENANT_ID");
  auto clientId = std::getenv("AZURE_CLIENT_ID");
  auto clientSecret = std::getenv("AZURE_CLIENT_SECRET");
  auto credential
      = std::make_shared<Azure::Identity::ClientSecretCredential>(tenantId, clientId, clientSecret);

  SecretClient secretClient(std::getenv("AZURE_KEYVAULT_URL"), credential);

  auto response = secretClient.GetSecret("testSecret");

  return 0;
}
