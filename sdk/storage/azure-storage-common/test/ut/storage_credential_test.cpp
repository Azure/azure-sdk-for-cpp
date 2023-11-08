// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include "test_base.hpp"

#include <azure/storage/common/storage_credential.hpp>

namespace Azure { namespace Storage { namespace Test {

  TEST(StorageCredentialTest, DefaultBlobHostCorrect)
  {
    EXPECT_EQ(
        Azure::Storage::_internal::ParseConnectionString(
            "DefaultEndpointsProtocol=https;AccountName=testaccount;AccountKey=testkey")
            .BlobServiceUrl.GetHost(),
        "testaccount.blob.core.windows.net");

    EXPECT_EQ(
        Azure::Storage::_internal::ParseConnectionString(
            "DefaultEndpointsProtocol=https;AccountName=testaccount;AccountKey=testkey;"
            "EndpointSuffix=core.windows.net")
            .BlobServiceUrl.GetHost(),
        "testaccount.blob.core.windows.net");

    EXPECT_EQ(
        Azure::Storage::_internal::ParseConnectionString(
            "DefaultEndpointsProtocol=https;AccountName=testaccount;AccountKey=testkey")
            .AccountKey,
        "testkey");

    EXPECT_EQ(
        Azure::Storage::_internal::ParseConnectionString(
            "DefaultEndpointsProtocol=https;AccountName=testaccount;AccountKey=testkey")
            .AccountName,
        "testaccount");

    EXPECT_EQ(
        Azure::Storage::_internal::ParseConnectionString(
            "DefaultEndpointsProtocol=https;AccountName=testaccount;AccountKey=testkey")
            .KeyCredential->AccountName,
        "testaccount");
  }

  TEST(StorageCredentialTest, DefaultTableHostCorrect)
  {
    EXPECT_EQ(
        Azure::Storage::_internal::ParseConnectionString(
            "DefaultEndpointsProtocol=https;AccountName=testaccount;AccountKey=testkey")
            .TableServiceUrl.GetHost(),
        "testaccount.table.core.windows.net");

    EXPECT_EQ(
        Azure::Storage::_internal::ParseConnectionString(
            "DefaultEndpointsProtocol=https;AccountName=testaccount;AccountKey=testkey;"
            "EndpointSuffix=core.windows.net")
            .TableServiceUrl.GetHost(),
        "testaccount.table.core.windows.net");

    EXPECT_EQ(
        Azure::Storage::_internal::ParseConnectionString(
            "DefaultEndpointsProtocol=https;AccountName=testaccount;AccountKey=testkey")
            .AccountKey,
        "testkey");

    EXPECT_EQ(
        Azure::Storage::_internal::ParseConnectionString(
            "DefaultEndpointsProtocol=https;AccountName=testaccount;AccountKey=testkey")
            .AccountName,
        "testaccount");

    EXPECT_EQ(
        Azure::Storage::_internal::ParseConnectionString(
            "DefaultEndpointsProtocol=https;AccountName=testaccount;AccountKey=testkey")
            .KeyCredential->AccountName,
        "testaccount");
  }

  TEST(StorageCredentialTest, DefaultQueueHostCorrect)
  {
    EXPECT_EQ(
        Azure::Storage::_internal::ParseConnectionString(
            "DefaultEndpointsProtocol=https;AccountName=testaccount;AccountKey=testkey")
            .QueueServiceUrl.GetHost(),
        "testaccount.queue.core.windows.net");

    EXPECT_EQ(
        Azure::Storage::_internal::ParseConnectionString(
            "DefaultEndpointsProtocol=https;AccountName=testaccount;AccountKey=testkey;"
            "EndpointSuffix=core.windows.net")
            .QueueServiceUrl.GetHost(),
        "testaccount.queue.core.windows.net");

    EXPECT_EQ(
        Azure::Storage::_internal::ParseConnectionString(
            "DefaultEndpointsProtocol=https;AccountName=testaccount;AccountKey=testkey")
            .AccountKey,
        "testkey");

    EXPECT_EQ(
        Azure::Storage::_internal::ParseConnectionString(
            "DefaultEndpointsProtocol=https;AccountName=testaccount;AccountKey=testkey")
            .AccountName,
        "testaccount");

    EXPECT_EQ(
        Azure::Storage::_internal::ParseConnectionString(
            "DefaultEndpointsProtocol=https;AccountName=testaccount;AccountKey=testkey")
            .KeyCredential->AccountName,
        "testaccount");
  }

  TEST(StorageCredentialTest, DefaultDataLakeHostCorrect)
  {
    EXPECT_EQ(
        Azure::Storage::_internal::ParseConnectionString(
            "DefaultEndpointsProtocol=https;AccountName=testaccount;AccountKey=testkey")
            .DataLakeServiceUrl.GetHost(),
        "testaccount.dfs.core.windows.net");

    EXPECT_EQ(
        Azure::Storage::_internal::ParseConnectionString(
            "DefaultEndpointsProtocol=https;AccountName=testaccount;AccountKey=testkey;"
            "EndpointSuffix=core.windows.net")
            .DataLakeServiceUrl.GetHost(),
        "testaccount.dfs.core.windows.net");

    EXPECT_EQ(
        Azure::Storage::_internal::ParseConnectionString(
            "DefaultEndpointsProtocol=https;AccountName=testaccount;AccountKey=testkey")
            .AccountKey,
        "testkey");

    EXPECT_EQ(
        Azure::Storage::_internal::ParseConnectionString(
            "DefaultEndpointsProtocol=https;AccountName=testaccount;AccountKey=testkey")
            .AccountName,
        "testaccount");

    EXPECT_EQ(
        Azure::Storage::_internal::ParseConnectionString(
            "DefaultEndpointsProtocol=https;AccountName=testaccount;AccountKey=testkey")
            .KeyCredential->AccountName,
        "testaccount");
  }

  TEST(StorageCredentialTest, DefaultFileHostCorrect)
  {
    EXPECT_EQ(
        Azure::Storage::_internal::ParseConnectionString(
            "DefaultEndpointsProtocol=https;AccountName=testaccount;AccountKey=testkey")
            .FileServiceUrl.GetHost(),
        "testaccount.file.core.windows.net");

    EXPECT_EQ(
        Azure::Storage::_internal::ParseConnectionString(
            "DefaultEndpointsProtocol=https;AccountName=testaccount;AccountKey=testkey;"
            "EndpointSuffix=core.windows.net")
            .FileServiceUrl.GetHost(),
        "testaccount.file.core.windows.net");

    EXPECT_EQ(
        Azure::Storage::_internal::ParseConnectionString(
            "DefaultEndpointsProtocol=https;AccountName=testaccount;AccountKey=testkey")
            .AccountKey,
        "testkey");

    EXPECT_EQ(
        Azure::Storage::_internal::ParseConnectionString(
            "DefaultEndpointsProtocol=https;AccountName=testaccount;AccountKey=testkey")
            .AccountName,
        "testaccount");

    EXPECT_EQ(
        Azure::Storage::_internal::ParseConnectionString(
            "DefaultEndpointsProtocol=https;AccountName=testaccount;AccountKey=testkey")
            .KeyCredential->AccountName,
        "testaccount");
  }
}}} // namespace Azure::Storage::Test
