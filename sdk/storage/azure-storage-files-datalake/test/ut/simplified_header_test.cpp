// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include <azure/storage/files/datalake.hpp>

#include <gtest/gtest.h>

namespace Azure { namespace Storage { namespace Test {

  TEST(SimplifiedHeader, StorageFilesDataLake)
  {
    Azure::Storage::Files::DataLake::DataLakeServiceClient serviceClient(
        "https://account.blob.core.windows.net");
    Azure::Storage::Files::DataLake::DataLakeFileSystemClient containerClient(
        "https://account.dfs.core.windows.net/container");
    Azure::Storage::Files::DataLake::DataLakePathClient pathClinet(
        "https://account.dfs.core.windows.net/container/path");
    Azure::Storage::Files::DataLake::DataLakeFileClient fileClinet(
        "https://account.dfs.core.windows.net/container/path");
    Azure::Storage::Files::DataLake::DataLakeDirectoryClient dirClinet(
        "https://account.dfs.core.windows.net/container/path");
    Azure::Storage::Files::DataLake::DataLakeLeaseClient leaseClient(
        containerClient,
        Azure::Storage::Files::DataLake::DataLakeLeaseClient::CreateUniqueLeaseId());

    Azure::Storage::Sas::DataLakeSasBuilder sasBuilder;

    StorageSharedKeyCredential keyCredential("account", "key");

    try
    {
    }
    catch (Azure::Storage::StorageException& e)
    {
      std::cout << e.what() << std::endl;
    }
  }

}}} // namespace Azure::Storage::Test
