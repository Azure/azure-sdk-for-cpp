// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include "test/ut/test_base.hpp"

#include <azure/storage/datamovement/blob_folder.hpp>

namespace Azure { namespace Storage { namespace Test {

  TEST_F(StorageTest, BlobFolderTest)
  {
    {
      const std::string serviceUrl = "https://account.blob.core.windows.net";
      const std::string containerUrl = serviceUrl + "/container";
      auto blobContainerClient = Blobs::BlobContainerClient(containerUrl);
      EXPECT_EQ(blobContainerClient.GetUrl(), containerUrl);

      {
        auto blobFolder = Blobs::BlobFolder(blobContainerClient, "folder1");
        EXPECT_EQ(blobFolder.GetUrl(), containerUrl + "/folder1");
        auto blobFolder2 = blobFolder.GetBlobFolder("folder2");
        EXPECT_EQ(blobFolder2.GetUrl(), containerUrl + "/folder1/folder2");
        auto blobClient = blobFolder.GetBlobClient("blobname");
        EXPECT_EQ(blobClient.GetUrl(), containerUrl + "/folder1/blobname");
        auto blobClient2 = blobFolder2.GetBlobClient("blobname2");
        EXPECT_EQ(blobClient2.GetUrl(), containerUrl + "/folder1/folder2/blobname2");
      }
      {
        auto blobFolder = Blobs::BlobFolder(blobContainerClient, "");
        EXPECT_EQ(blobFolder.GetUrl(), containerUrl + "/");

        auto blobFolder2 = blobFolder.GetBlobFolder("folder");
        EXPECT_EQ(blobFolder2.GetUrl(), containerUrl + "/folder");
        auto blobClient = blobFolder.GetBlobClient("blobname");
        EXPECT_EQ(blobClient.GetUrl(), containerUrl + "/blobname");
      }
    }
    {

      const std::string serviceUrl = "https://account.blob.core.windows.net";
      const std::string containerUrl = serviceUrl + "/container/";
      auto blobContainerClient = Blobs::BlobContainerClient(containerUrl);
      EXPECT_EQ(blobContainerClient.GetUrl(), containerUrl);

      {
        auto blobFolder = Blobs::BlobFolder(blobContainerClient, "folder1");
        EXPECT_EQ(blobFolder.GetUrl(), containerUrl + "folder1");
        auto blobFolder2 = blobFolder.GetBlobFolder("folder2");
        EXPECT_EQ(blobFolder2.GetUrl(), containerUrl + "folder1/folder2");
        auto blobClient = blobFolder.GetBlobClient("blobname");
        EXPECT_EQ(blobClient.GetUrl(), containerUrl + "folder1/blobname");
        auto blobClient2 = blobFolder2.GetBlobClient("blobname2");
        EXPECT_EQ(blobClient2.GetUrl(), containerUrl + "folder1/folder2/blobname2");
      }
      {
        auto blobFolder = Blobs::BlobFolder(blobContainerClient, "");
        EXPECT_EQ(blobFolder.GetUrl(), containerUrl);

        auto blobFolder2 = blobFolder.GetBlobFolder("folder");
        EXPECT_EQ(blobFolder2.GetUrl(), containerUrl + "folder");
        auto blobClient = blobFolder.GetBlobClient("blobname");
        EXPECT_EQ(blobClient.GetUrl(), containerUrl + "blobname");
      }
    }
  }

}}} // namespace Azure::Storage::Test
