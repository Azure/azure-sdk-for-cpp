// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

/**
 * @brief This sample provides smoke test for the sdks to ensure side by side works properly
 *
 */

#include "get_env.hpp"

#include <azure/core.hpp>
#include <azure/identity.hpp>
#include <azure/keyvault/keyvault_certificates.hpp>
#include <azure/keyvault/keyvault_keys.hpp>
#include <azure/keyvault/keyvault_secrets.hpp>
#include <azure/storage/blobs.hpp>
#include <azure/storage/files/datalake.hpp>
#include <azure/storage/files/shares.hpp>
#include <azure/storage/queues.hpp>

using namespace Azure::Security::KeyVault::Keys;
using namespace Azure::Security::KeyVault::Secrets;
using namespace Azure::Security::KeyVault::Certificates;
using namespace Azure::Storage::Blobs;
using namespace Azure::Storage::Queues;
using namespace Azure::Storage::Files::DataLake;
using namespace Azure::Storage::Files::Shares;

int main()
{
  const std::string tenantId = "tenant";
  const std::string clientId = "client";
  const std::string clientSecret = "secret";
  const std::string leaseID = "leaseID";
  const std::string storageUrl = "https://blob.com";
  auto credential
      = std::make_shared<Azure::Identity::ClientSecretCredential>(tenantId, clientId, clientSecret);
  
  // instantiate the clients
  
  // keyvault
  KeyClient keyClient(std::getenv("AZURE_KEYVAULT_URL"), credential);
  SecretClient secretClient(std::getenv("AZURE_KEYVAULT_URL"), credential);
  CertificateClient certificateClient(std::getenv("AZURE_KEYVAULT_URL"), credential);

  // Storage
  BlobClient blobClient(storageUrl);
  QueueClient queueClient(storageUrl);

  DataLakeDirectoryClient directoryClient(storageUrl);
  DataLakeFileClient fileClient(storageUrl);
  DataLakeFileSystemClient fileSystemClient(storageUrl);
  DataLakePathClient pathClient(storageUrl);
  DataLakeLeaseClient leaseClient(pathClient, leaseID);
  DataLakeServiceClient serviceClient(storageUrl);

  ShareClient shareClient(storageUrl);
  ShareDirectoryClient shareDirectoryClient(storageUrl);
  ShareFileClient shareFileClient(storageUrl);
  ShareLeaseClient shareLeaseClient(shareFileClient, leaseID);
  ShareServiceClient shareServiceClient(storageUrl);

  return 0;
}

