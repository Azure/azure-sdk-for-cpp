// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include <iostream>

#include <azure/storage/blobs.hpp>

int main()
{
  using namespace Azure::Storage::Blobs;

  const std::string connectionString = "";
  const std::string containerName = "sample-container";
  const std::string blobName = "sample-blob";
  const std::string blobContent = "Hello Azure!";

  {
    // Create some containers and blobs for test
    for (int i = 0; i < 2; ++i)
    {
      auto containerClient = BlobContainerClient::CreateFromConnectionString(
          connectionString, containerName + std::to_string(i));
      containerClient.CreateIfNotExists();
      for (int j = 0; j < 3; ++j)
      {
        BlockBlobClient blobClient
            = containerClient.GetBlockBlobClient(blobName + std::to_string(j));
        blobClient.UploadFrom(
            reinterpret_cast<const uint8_t*>(blobContent.data()), blobContent.size());
      }
    }
  }

  auto serviceClient = BlobServiceClient::CreateFromConnectionString(connectionString);

  for (auto containerPage = serviceClient.ListBlobContainers(); containerPage.HasPage();
       containerPage.MoveToNextPage())
  {
    for (auto& container : containerPage.BlobContainers)
    {
      // Below is what you want to do with each container
      std::cout << "blob container: " << container.Name << std::endl;
      for (auto blobPage = serviceClient.GetBlobContainerClient(container.Name).ListBlobs();
           blobPage.HasPage();
           blobPage.MoveToNextPage())
      {
        for (auto& blob : blobPage.Blobs)
        {
          // Below is what you want to do with each blob
          std::cout << "    blob: " << blob.Name << std::endl;
        }
      }
    }
  }

  return 0;
}
