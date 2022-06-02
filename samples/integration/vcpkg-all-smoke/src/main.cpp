// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

/**
 * @brief This sample provides smoke test for the sdks to ensure side by side works properly
 *
 */

#include "get_env.hpp"
#include <azure/attestation.hpp>
#include <azure/core.hpp>
#include <azure/core/internal/json/json.hpp>
#include <azure/identity.hpp>
#include <azure/keyvault/certificates.hpp>
#include <azure/keyvault/keys.hpp>
#include <azure/keyvault/secrets.hpp>
#include <azure/storage/blobs.hpp>
#include <azure/storage/files/datalake.hpp>
#include <azure/storage/files/shares.hpp>
#include <azure/storage/queues.hpp>
#include <iostream>

using namespace Azure::Security::KeyVault::Keys;
using namespace Azure::Security::KeyVault::Secrets;
using namespace Azure::Security::KeyVault::Certificates;
using namespace Azure::Storage::Blobs;
using namespace Azure::Storage::Queues;
using namespace Azure::Storage::Files::DataLake;
using namespace Azure::Storage::Files::Shares;
using namespace Azure::Security::Attestation;

int main()
{
  auto tenantId = std::getenv("AZURE_TENANT_ID");
  auto clientId = std::getenv("AZURE_CLIENT_ID");
  auto clientSecret = std::getenv("AZURE_CLIENT_SECRET");
  const std::string leaseID = "leaseID";
  const std::string smokeUrl = "https://blob.com";
  // Creating an attestation service instance requires contacting the attestation service (to
  // retrieve validation collateral). Use the West US Shared client (which should always be
  // available) as an anonymous service instance.
  const std::string attestationUrl = "https://sharedwus.wus.attest.azure.net";

  auto credential
      = std::make_shared<Azure::Identity::ClientSecretCredential>(tenantId, clientId, clientSecret);

  // instantiate the clients
  try
  {
    std::cout << "Creating Keyvault Clients" << std::endl;
    // keyvault
    KeyClient keyClient(smokeUrl, credential);
    SecretClient secretClient(smokeUrl, credential);
    CertificateClient certificateClient(smokeUrl, credential);

    std::cout << "Creating Storage Clients" << std::endl;
    // Storage
    BlobClient blobClient(smokeUrl);
    QueueClient queueClient(smokeUrl);

    std::cout << "Creating Storage Datalake Clients" << std::endl;
    DataLakeDirectoryClient directoryClient(smokeUrl);
    DataLakeFileClient fileClient(smokeUrl);
    DataLakeFileSystemClient fileSystemClient(smokeUrl);
    DataLakePathClient pathClient(smokeUrl);
    DataLakeLeaseClient leaseClient(pathClient, leaseID);
    DataLakeServiceClient serviceClient(smokeUrl);

    std::cout << "Creating Storage Share Clients" << std::endl;
    ShareClient shareClient(smokeUrl);
    ShareDirectoryClient shareDirectoryClient(smokeUrl);
    ShareFileClient shareFileClient(smokeUrl);
    ShareLeaseClient shareLeaseClient(shareFileClient, leaseID);
    ShareServiceClient shareServiceClient(smokeUrl);

    // Attestation
    std::cout << "Creating Attestation Clients" << std::endl;

    AttestationAdministrationClient attestationAdminClient(
        AttestationAdministrationClient::Create(attestationUrl, credential));

    AttestationClient attestationClient(AttestationClient::Create(attestationUrl));

    std::cout << "Successfully Created the Clients" << std::endl;
  }
  catch (std::exception const& exception)
  {
    std::cout << "Exception: " << exception.what() << std::endl;
    return 1;
  }

  return 0;
}
