// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include <azure/storage/files/shares.hpp>

#include <gtest/gtest.h>

namespace Azure { namespace Storage { namespace Test {

  TEST(SimplifiedHeader, StorageFilesShares)
  {
    Azure::Storage::Files::Shares::ShareServiceClient serviceClient(
        "https://account.blob.core.windows.net");
    Azure::Storage::Files::Shares::ShareClient shareClient(
        "https://account.file.core.windows.net/share");
    Azure::Storage::Files::Shares::ShareFileClient fileClient(
        "https://account.file.core.windows.net/share/file");
    Azure::Storage::Files::Shares::ShareDirectoryClient dirClient(
        "https://account.file.core.windows.net/share/dir");

    Azure::Storage::Files::Shares::ShareLeaseClient leaseClient(
        shareClient, Azure::Storage::Files::Shares::ShareLeaseClient::CreateUniqueLeaseId());

    Azure::Storage::Sas::ShareSasBuilder sasBuilder;

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


