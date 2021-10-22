// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include <iostream>

#include <azure/storage/blobs.hpp>

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

  containerClient.CreateIfNotExists();

  BlockBlobClient blobClient = containerClient.GetBlockBlobClient(blobName);
  

  std::vector<uint8_t> buffer(blobContent.begin(), blobContent.end());
  blobClient.UploadFrom(buffer.data(), buffer.size());

  Azure::Storage::Metadata blobMetadata = {{"key1", "value1"}, {"key2", "value2"}};
  blobClient.SetMetadata(blobMetadata);

  auto properties = blobClient.GetProperties().Value;
  for (auto metadata : properties.Metadata)
  {
    std::cout << metadata.first << ":" << metadata.second << std::endl;
  }
  // We know blob size is small, so it's safe to cast here.
  buffer.resize(static_cast<size_t>(properties.BlobSize));

  blobClient.DownloadTo(buffer.data(), buffer.size());

  std::cout << std::string(buffer.begin(), buffer.end()) << std::endl;
}
