// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include "blobs/blob.hpp"

#include <fstream>
#include <iostream>

int main()
{
  using namespace Azure::Storage::Blobs;
  BlobClient blob_client("https://targettest.blob.core.windows.net/container1/file_8M");
  auto properties = blob_client.GetProperties();
  std::cout << properties.ContentLength << std::endl;

  auto downloaded_data = blob_client.Download();

  return 0;
}