// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include "blobs/blob.hpp"
#include "samples_common.hpp"

#include <iostream>

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

  auto blobContentStream = Azure::Storage::CreateMemoryStream(
      reinterpret_cast<const uint8_t*>(blobContent.data()), blobContent.length());
  blobClient.Upload(std::move(blobContentStream));

  std::map<std::string, std::string> blobMetadata = {{"key1", "value1"}, {"key2", "value2"}};
  blobClient.SetMetadata(blobMetadata);

  auto blobDownloadContent = blobClient.Download();
  blobContent.resize(static_cast<std::size_t>(blobDownloadContent.BodyStream->Length()));
  std::size_t offset = 0;
  while (true)
  {
    auto bytesRead = blobDownloadContent.BodyStream->Read(
        reinterpret_cast<uint8_t*>(&blobContent[offset]), blobContent.length() - offset);
    offset += static_cast<std::size_t>(bytesRead);
    if (bytesRead == 0 || offset == blobContent.length())
    {
      break;
    }
  }

  std::cout << blobContent << std::endl;

  auto properties = blobClient.GetProperties();
  for (auto metadata : properties.Metadata)
  {
    std::cout << metadata.first << ":" << metadata.second << std::endl;
  }
}
