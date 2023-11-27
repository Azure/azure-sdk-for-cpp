// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include <azure/storage/blobs.hpp>
#include <azure/storage/common/storage_exception.hpp>

#include <cstdio>
#include <iostream>
#include <stdexcept>

std::string GetConnectionString()
{
  const static std::string ConnectionString = "";

  if (!ConnectionString.empty())
  {
    return ConnectionString;
  }
  const static std::string envConnectionString = std::getenv("AZURE_STORAGE_CONNECTION_STRING");
  if (!envConnectionString.empty())
  {
    return envConnectionString;
  }
  throw std::runtime_error("Cannot find connection string.");
}

std::string GetAccountName()
{
  const auto connectionString = GetConnectionString();
  const std::string property = "AccountName=";
  auto pos1 = connectionString.find(property);
  auto pos2 = connectionString.find(";", pos1);
  return connectionString.substr(pos1 + property.length(), pos2 - pos1 - property.length());
}

std::string GetAccountKey()
{
  const auto connectionString = GetConnectionString();
  const std::string property = "AccountKey=";
  auto pos1 = connectionString.find(property);
  auto pos2 = connectionString.find(";", pos1);
  return connectionString.substr(pos1 + property.length(), pos2 - pos1 - property.length());
}

int main()
{
  using namespace Azure::Storage::Blobs;

  const std::string containerName = "sample-container";
  const std::string blobName = "sample-blob";
  const std::string blobContent = "Hello Azure!";

  // Create a container and a blob for test
  {
    auto credential = std::make_shared<Azure::Storage::StorageSharedKeyCredential>(
        GetAccountName(), GetAccountKey());
    auto containerClient = BlobContainerClient(
        "https://" + GetAccountName() + ".blob.core.windows.net/" + containerName, credential);
    containerClient.CreateIfNotExists();
    BlockBlobClient blobClient = containerClient.GetBlockBlobClient(blobName);
    blobClient.UploadFrom(reinterpret_cast<const uint8_t*>(blobContent.data()), blobContent.size());
  }

  Azure::Storage::Sas::BlobSasBuilder sasBuilder;
  sasBuilder.ExpiresOn = std::chrono::system_clock::now() + std::chrono::minutes(60);
  sasBuilder.BlobContainerName = containerName;
  sasBuilder.BlobName = blobName;
  sasBuilder.Resource = Azure::Storage::Sas::BlobSasResource::Blob;
  // Read permission only
  sasBuilder.SetPermissions(Azure::Storage::Sas::BlobSasPermissions::Read);

  std::string sasToken = sasBuilder.GenerateSasToken(
      Azure::Storage::StorageSharedKeyCredential(GetAccountName(), GetAccountKey()));

  auto blobClient = BlobClient(
      "https://" + GetAccountName() + ".blob.core.windows.net/" + containerName + "/" + blobName
      + sasToken);

  // We can read the blob
  auto properties = blobClient.GetProperties().Value;

  try
  {

    Azure::Storage::Metadata metadata;
    // But we cannot write, this will throw
    blobClient.SetMetadata(metadata);
    // Never reach here
    std::abort();
  }
  catch (const Azure::Storage::StorageException&)
  {
  }
}
