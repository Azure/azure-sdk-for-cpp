// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include "get_env.hpp"
#include <cstdio>
#include <iostream>
#include <stdexcept>

#include <azure/storage/blobs.hpp>

std::string GetConnectionString();

/**
 * @brief Creates a SaS for an specific container with all permissions for an hour.
 *
 * Required env var:
 *
 * - AZURE_CPP_VCPKG_CACHE_ACCOUNT_KEY
 *   - App throws if the env var is not found
 *
 * Default values:
 *
 * - AccountName: cppvcpkgcache
 *   - Override using env var: AZURE_CPP_VCPKG_CACHE_ACCOUNT_NAME
 *
 * - ContainerName: public-vcpkg-container
 *   - Override using env var: AZURE_CPP_VCPKG_CACHE_ACCOUNT_CONTAINER
 *
 * @return int
 */
int main()
{
  using namespace Azure::Storage::Blobs;
  using namespace Azure::Storage::_internal;

  std::string const connectionString(GetConnectionString());
  ConnectionStringParts const parsedConnectionString(ParseConnectionString(GetConnectionString()));
  std::string const accountName(parsedConnectionString.AccountName);
  std::string const accountKey(parsedConnectionString.AccountKey);

  std::string containerName = "public-vcpkg-container";
  auto const containerNamePtr = std::getenv("AZURE_CPP_VCPKG_CACHE_ACCOUNT_CONTAINER");
  if (containerNamePtr)
  {
    containerName = std::string(containerNamePtr);
  }

  Azure::Storage::Sas::BlobSasBuilder sasBuilder;
  sasBuilder.ExpiresOn = std::chrono::system_clock::now() + std::chrono::minutes(60);
  sasBuilder.BlobContainerName = containerName;
  sasBuilder.Resource = Azure::Storage::Sas::BlobSasResource::BlobContainer;
  // All permissions
  sasBuilder.SetPermissions(Azure::Storage::Sas::BlobContainerSasPermissions::All);

  std::string sasToken = sasBuilder.GenerateSasToken(
      Azure::Storage::StorageSharedKeyCredential(accountName, accountKey));

  // display
  std::cout << sasToken << std::endl;

  // test a simple list operation
  auto containerClient = BlobContainerClient(
      "https://" + accountName + ".blob.core.windows.net/" + containerName + sasToken);
  // will throw for invalid SaS
  containerClient.ListBlobs();

  return 0;
}

std::string GetConnectionString()
{
  auto const envConnectionKeyPtr = std::getenv("AZURE_CPP_VCPKG_CACHE_ACCOUNT_KEY");
  if (!envConnectionKeyPtr)
  {
    throw std::runtime_error("Cannot find AZURE_CPP_VCPKG_CACHE_ACCOUNT_KEY.");
  }
  std::string const accountKey(envConnectionKeyPtr);

  std::string accountName("cppvcpkgcache");
  auto const accountNamePtr = std::getenv("AZURE_CPP_VCPKG_CACHE_ACCOUNT_NAME");
  if (accountNamePtr)
  {
    accountName = std::string(accountNamePtr);
  }

  return std::string(
      "DefaultEndpointsProtocol=https;AccountName=" + accountName + ";AccountKey=" + accountKey
      + ";EndpointSuffix=core.windows.net");
}
