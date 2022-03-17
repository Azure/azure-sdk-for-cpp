// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

/**
 * @brief This sample provides smoke test for the sdks to ensure side by side works properly
 *
 * @remark The following environment variables must be set before running the sample.
 * - AZURE_KEYVAULT_URL:  To the Key Vault account URL.
 * - AZURE_TENANT_ID:     Tenant ID for the Azure account.
 * - AZURE_CLIENT_ID:     The Client ID to authenticate the request.
 * - AZURE_CLIENT_SECRET: The client secret.
 *
 */

#include "get_env.hpp"

#include <azure/core.hpp>
#include <azure/identity.hpp>
#include <azure/keyvault/keyvault_certificates.hpp>
#include <azure/keyvault/keyvault_keys.hpp>
#include <azure/keyvault/keyvault_secrets.hpp>
#include <azure/storage/blobs.hpp>
#include <azure/storage/queues.hpp>
#include <azure/storage/files/datalake.hpp>
#include <azure/storage/files/shares.hpp>

#include <chrono>
#include <iostream>
#include <memory>
#include <thread>

using namespace Azure::Security::KeyVault::Keys;
using namespace Azure::Security::KeyVault::Secrets;
using namespace Azure::Security::KeyVault::Certificates;
using namespace Azure::Storage::Blobs;
using namespace Azure::Storage::Queues;
using namespace Azure::Storage::Files::DataLake;
using namespace Azure::Storage::Files::Shares;

int main()
{
  auto tenantId = "tenant";
  auto clientId = "client";
  auto clientSecret = "secret";
  auto credential
      = std::make_shared<Azure::Identity::ClientSecretCredential>(tenantId, clientId, clientSecret);
 // auto connString = "DefaultEndpointsProtocol=https;AccountName=notReal;AccountKey="
 //                   "333333333333333333333333333333333333333333333333333333333333333333333333333333"
 //                   "3333333333;EndpointSuffix=core.windows.net";
  auto storageUrl = "https://blob.com";
  // instantiate the clients
  KeyClient keyClient(std::getenv("AZURE_KEYVAULT_URL"), credential);
  SecretClient secretClient(std::getenv("AZURE_KEYVAULT_URL"), credential);
  CertificateClient certificateClient(std::getenv("AZURE_KEYVAULT_URL"), credential);
  BlobClient blobClient(storageUrl);
  QueueClient queueClient(storageUrl);

  return 0;
}
