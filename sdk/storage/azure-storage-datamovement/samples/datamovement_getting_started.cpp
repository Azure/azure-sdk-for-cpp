// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include "get_env.hpp"

#undef CreateDirectory

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
  using namespace Azure::Storage;
  using namespace Azure::Storage::Blobs;

  const std::string containerName = "sample-container";
  const std::string blobName = "sample-blob";
  const std::string localFile = "sample-localfile";
  const std::string localDirectory = "sample-localdir";

  // create local file for testing
  const std::string fileContent = "Hello Azure!";
  std::ofstream fOut(localFile, std::ofstream::binary);
  fOut.write(fileContent.data(), fileContent.length());
  fOut.close();
  _internal::CreateDirectory(localDirectory);
  fOut.open(localDirectory + "/" + localFile, std::ofstream::binary);
  fOut.write(fileContent.data(), fileContent.length());
  fOut.close();

  BlobTransferManager m;

  auto blobContainerClient
      = BlobContainerClient::CreateFromConnectionString(GetConnectionString(), containerName);
  blobContainerClient.CreateIfNotExists();
  auto blobClient = blobContainerClient.GetBlobClient(blobName);

  auto job1 = m.ScheduleUpload(localFile, blobClient);
  std::cout << job1.Id << std::endl;
  std::cout << job1.SourceUrl << " -> " << job1.DestinationUrl << std::endl;

  auto blobFolder = BlobFolder(blobContainerClient, localDirectory);
  auto job2 = m.ScheduleUploadDirectory(localDirectory, blobFolder);
  std::cout << job2.Id << std::endl;
  std::cout << job2.SourceUrl << " -> " << job2.DestinationUrl << std::endl;

  auto job1Status = job1.WaitHandle.get();
  if (job1Status == JobStatus::Succeeded)
  {
    std::cout << "job1 succeeded" << std::endl;
  }
  auto job2Status = job2.WaitHandle.get();
  if (job2Status == JobStatus::Succeeded)
  {
    std::cout << "job2 succeeded" << std::endl;
  }

  auto job3 = m.ScheduleDownload(blobClient, localFile + "_2");
  std::cout << job3.Id << std::endl;
  std::cout << job3.SourceUrl << " -> " << job3.DestinationUrl << std::endl;
  auto job3Status = job3.WaitHandle.get();
  if (job3Status == JobStatus::Succeeded)
  {
    std::cout << "job3 succeeded" << std::endl;
  }
  return 0;
}
