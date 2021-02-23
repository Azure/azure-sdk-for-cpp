// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

/**
 * @file
 * @brief Application that consumes the Azure SDK for C++.
 *
 * @remark Set environment variable `STORAGE_CONNECTION_STRING` before running the application.
 *
 */

#include <azure/storage/blobs.hpp>

#include <exception>
#include <iostream>

using namespace Azure::Storage::Blobs;

int main(int argc, char* argv[])
{
  (void)argc;
  (void)argv;

  /**************** Container sdk client ************************/
  /****************   Create container  ************************/
  try
  {
    auto containerClient = BlobContainerClient::CreateFromConnectionString(
        std::getenv("STORAGE_CONNECTION_STRING"), "sample");

    containerClient.CreateIfNotExists();

    /**************** Container sdk client ************************/
    /****************      list Blobs (one page) ******************/
    auto response = containerClient.ListBlobsSinglePage();
    auto blobListPage = response.ExtractValue();
    for (auto blob : blobListPage.Items)
    {
      std::cout << blob.Name << std::endl;
    }
  }
  catch (const std::exception& ex)
  {
    std::cout << ex.what();
    return 1;
  }

  return 0;
}
