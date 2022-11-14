//  Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include "get_env.hpp"

#include <cstdio>
#include <iostream>
#include <stdexcept>

#include <azure/storage/blobs.hpp>

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

  return 0;
}
