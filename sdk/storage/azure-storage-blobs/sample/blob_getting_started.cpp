// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include <iostream>

#include "azure/storage/blobs.hpp"
#include "samples_common.hpp"

SAMPLE(BlobsGettingStarted, BlobsGettingStarted)
void BlobsGettingStarted()
{
  using namespace Azure::Storage::Blobs;

  std::string containerName = "sample-container";
  std::string blobName = "sample-blob";
  std::string blobContent = "Hello Azure!";

  auto containerClient
      = BlobContainerClient::CreateFromConnectionString(GetConnectionString(), containerName);
  try
  {
    containerClient.Create();
  }
  catch (std::runtime_error& e)
  {
    // The container may already exist
    std::cout << e.what() << std::endl;
  }

  BlockBlobClient blobClient = containerClient.GetBlockBlobClient(blobName);

  blobClient.UploadFrom(reinterpret_cast<const uint8_t*>(blobContent.data()), blobContent.size());

  std::map<std::string, std::string> blobMetadata = {{"key1", "value1"}, {"key2", "value2"}};
  blobClient.SetMetadata(blobMetadata);

  auto properties = *blobClient.GetProperties();
  for (auto metadata : properties.Metadata)
  {
    std::cout << metadata.first << ":" << metadata.second << std::endl;
  }
  blobContent.resize(static_cast<std::size_t>(properties.ContentLength));

  blobClient.DownloadTo(reinterpret_cast<uint8_t*>(&blobContent[0]), blobContent.size());

  std::cout << blobContent << std::endl;
}
