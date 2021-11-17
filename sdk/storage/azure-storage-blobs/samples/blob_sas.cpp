// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include <iostream>

#include <azure/storage/blobs.hpp>

int main()
{
  using namespace Azure::Storage::Blobs;

  const std::string accountName = "";
  const std::string accountKey = "";
  const std::string containerName = "sample-container";
  const std::string blobName = "sample-blob";
  const std::string blobContent = "Hello Azure!";

  // Create a container and a blob for test
  {
    auto credential
        = std::make_shared<Azure::Storage::StorageSharedKeyCredential>(accountName, accountKey);
    auto containerClient = BlobContainerClient(
        "https://" + accountName + ".blob.core.windows.net/" + containerName, credential);
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
      Azure::Storage::StorageSharedKeyCredential(accountName, accountKey));

  auto blobClient = BlobClient(
      "https://" + accountName + ".blob.core.windows.net/" + containerName + "/" + blobName
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
