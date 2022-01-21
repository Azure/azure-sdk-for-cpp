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

const char* GetConnectionString()
{
#if defined(UWP)
  // UWP platform does not support environment variables.
  // Implement some other way to get these values, such as reading them from a config file.
  // Do not put values directly in code, especially secrets.
  throw std::exception();
#else
  return std::getenv("STORAGE_CONNECTION_STRING");
#endif
}

int main(int argc, char* argv[])
{
  (void)argc;
  (void)argv;

  /**************** Container SDK client ************************/
  /****************   Create container  ************************/
  try
  {
    auto containerClient
        = BlobContainerClient::CreateFromConnectionString(GetConnectionString(), "sample");

    containerClient.CreateIfNotExists();

    /**************** Container SDK client ************************/
    /****************      list blobs (one page) ******************/
    auto response = containerClient.ListBlobsSinglePage();
    auto blobListPage = response.Value;
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
