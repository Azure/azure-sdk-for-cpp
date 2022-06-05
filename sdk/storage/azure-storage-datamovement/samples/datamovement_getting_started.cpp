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
  const static std::string envConnectionString = "DefaultEndpointsProtocol=https;AccountName=emmahktest;AccountKey=9J/G/orROAbao1ujMkn4+5l+ZosLXtcY9YXnuX+urcw5tqtkPBm/jf+7w7rb5zvBVdxo1T8DW/vkf6o4KgZgiw==;EndpointSuffix=core.windows.net";
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

  const std::string containerName = "acontainer111";
  const std::string blobName = "blockblob";
  const std::string localFile = "D:\\work\\tempfile";
  const std::string localDirectory = "sample-localdir";

  BlobTransferManager m;

  auto blobContainerClient
      = BlobContainerClient::CreateFromConnectionString(GetConnectionString(), containerName);
  blobContainerClient.CreateIfNotExists();

  auto blobFolder = BlobFolder(blobContainerClient, "testfolder");
  auto job2 = m.ScheduleDownloadDirectory(blobFolder, "D:\\work\\testfolder1");
  std::cout << job2.Id << std::endl;
  std::cout << job2.SourceUrl << " -> " << job2.DestinationUrl << std::endl;

  auto job2Status = job2.WaitHandle.get();
  if (job2Status == JobStatus::Succeeded)
  {
    std::cout << "job2 succeeded" << std::endl;
  }
  return 0;
}
