// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include <azure/core/context.hpp>
#include <azure/storage/blobs.hpp>

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

int main()
{
  using namespace Azure::Storage::Blobs;

  const std::string containerName = "sample-container";
  const std::string blobName = "sample-blob";
  const std::string blobContent = "Hello Azure!";

  {
    // Create some containers and blobs for test
    for (int i = 0; i < 2; ++i)
    {
      // @begin_snippet: CreateBlobContext
      Azure::Core::Context cancelledIn5s = Azure::Core::Context::CreateWithDeadline(
          std::chrono::system_clock::now() + std::chrono::seconds(5));

      auto containerClient = BlobContainerClient::CreateFromConnectionString(
          GetConnectionString(), containerName + std::to_string(i));
      containerClient.CreateIfNotExists({}, cancelledIn5s);
      for (int j = 0; j < 3; ++j)
      {
        BlockBlobClient blobClient
            = containerClient.GetBlockBlobClient(blobName + std::to_string(j));
        blobClient.UploadFrom(
            reinterpret_cast<const uint8_t*>(blobContent.data()),
            blobContent.size(),
            {},
            cancelledIn5s);
      }
      // @end_snippet: CreateBlobContext
    }
  }

  auto serviceClient = BlobServiceClient::CreateFromConnectionString(GetConnectionString());

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
