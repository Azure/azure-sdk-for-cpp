// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include "get_env.hpp"

#include <iostream>

#include <azure/storage/datamovement/blob_transfer_manager.hpp>

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
  const std::string localFile = "sample-localfile";
  const std::string localDirectory = "sample-localdir";

  BlobTransferManager m;

  auto blobContainerClient
      = BlobContainerClient::CreateFromConnectionString(GetConnectionString(), containerName);
  blobContainerClient.CreateIfNotExists();
  auto blobClient = blobContainerClient.GetBlobClient(blobName);

  auto job = m.ScheduleUpload(localFile, blobClient);
  std::cout << job.JobId << std::endl;
  std::cout << job.SourceUrl << " -> " << job.DestinationUrl << std::endl;

  auto blobFolder = BlobFolder::CreateFromConnectionString(
      GetConnectionString(), containerName, localDirectory);
  auto job2 = m.ScheduleUploadDirectory(localDirectory, blobFolder);
  std::cout << job2.JobId << std::endl;
  std::cout << job2.SourceUrl << " -> " << job2.DestinationUrl << std::endl;
  auto c = getchar();
  (void)c;

  return 0;
}
