// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

/**
 * @brief Performs  upload and download for n times trying to catch long running bugs.
 * Originally set to use 50Mb for 2000 times, which takes ~2.5hours to complete.
 *
 */

#ifdef _MSC_VER
// this option is used to allow the application to use std::getenv without getting a compilation
// warning about it on MSVC.
#define _CRT_SECURE_NO_WARNINGS
#endif

#include <iostream>

#include <azure/storage/blobs/blob.hpp>

#define BLOB_SIZE 50 * 1024ULL * 1024
#define REPEAT_FOR 2000
#define CONCURRENCY 16

int main()
{
  using namespace Azure::Storage::Blobs;

  std::string containerName = "sample-container";
  std::string blobName = "sample-blob";
  std::string blobContent;
  blobContent.resize(BLOB_SIZE, 'c');

  std::string connString(std::getenv("STORAGE_CONNECTION_STRING"));

  auto containerClient = BlobContainerClient::CreateFromConnectionString(connString, containerName);
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

  for (int i = 0; i < REPEAT_FOR; ++i)
  {
    {
      UploadBlockBlobFromOptions options;
      options.Concurrency = CONCURRENCY;
      blobClient.UploadFrom(
          reinterpret_cast<const uint8_t*>(blobContent.data()), blobContent.size(), options);
    }

    {
      std::string download;
      download.resize(BLOB_SIZE, '.');
      DownloadBlobToOptions options;
      options.Concurrency = CONCURRENCY;
      blobClient.DownloadTo(reinterpret_cast<uint8_t*>(&download[0]), download.size(), options);

      // make sure download content is the one expected
      if (download != blobContent)
      {
        std::cout << "Downloaded content is not the same" << std::endl;
        return 1;
      }
    }

    std::cout << i << std::endl;
  }
}
