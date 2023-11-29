// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include <azure/storage/blobs.hpp>

#include <gtest/gtest.h>

namespace Azure { namespace Storage { namespace Test {

  TEST(SimplifiedHeader, StorageBlobs)
  {
    Azure::Storage::Blobs::BlobServiceClient serviceClient("https://account.blob.core.windows.net");
    Azure::Storage::Blobs::BlobContainerClient containerClient(
        "https://account.blob.core.windows.net/container");
    Azure::Storage::Blobs::BlobClient blobClinet(
        "https://account.blob.core.windows.net/container/blob");
    Azure::Storage::Blobs::BlockBlobClient blockBlobClinet(
        "https://account.blob.core.windows.net/container/blob");
    Azure::Storage::Blobs::PageBlobClient pageBlobClinet(
        "https://account.blob.core.windows.net/container/blob");
    Azure::Storage::Blobs::AppendBlobClient appendBlobClinet(
        "https://account.blob.core.windows.net/container/blob");
    Azure::Storage::Blobs::BlobLeaseClient leaseClient(
        containerClient, Azure::Storage::Blobs::BlobLeaseClient::CreateUniqueLeaseId());

    Azure::Storage::Sas::BlobSasBuilder sasBuilder;

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


