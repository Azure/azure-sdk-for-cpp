// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

/**
 * @brief This sample provides the code implementation to use the Storage blobs SDK client for C++
 * to create a container and upload a blob to it.
 *
 * @remark The following environment variables must be set before running the sample.
 * - AZURE_STORAGE_CONNECTION_STRING:  Set it Azure Storage connection string.
 *
 */

#include <get_env.hpp>

#include <iostream>

#include <azure/storage/blobs.hpp>

const std::string& GetConnectionString();

int main()
{
  using namespace Azure::Storage::Blobs;

  std::string containerName = "sample-container";
  std::string blobName = "sample-blob";
  std::string blobContent = "Hello Azure!";

  auto containerClient
      = BlobContainerClient::CreateFromConnectionString(GetConnectionString(), containerName);

  containerClient.CreateIfNotExists();

  BlockBlobClient blobClient = containerClient.GetBlockBlobClient(blobName);

  blobClient.UploadFrom(reinterpret_cast<const uint8_t*>(blobContent.data()), blobContent.size());

  Azure::Storage::Metadata blobMetadata = {{"key1", "value1"}, {"key2", "value2"}};
  blobClient.SetMetadata(blobMetadata);

  auto properties = blobClient.GetProperties().Value;
  for (auto metadata : properties.Metadata)
  {
    std::cout << metadata.first << ":" << metadata.second << std::endl;
  }
  blobContent.resize(static_cast<size_t>(properties.BlobSize));

  blobClient.DownloadTo(reinterpret_cast<uint8_t*>(&blobContent[0]), blobContent.size());

  std::cout << blobContent << std::endl;

  return 0;
}

const std::string& GetConnectionString()
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
  throw std::runtime_error("Cannot find connection string");
}
