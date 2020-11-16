// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include "azure/storage/common/storage_credential.hpp"
#include "test_base.hpp"

namespace Azure { namespace Storage { namespace Test {

  TEST(StorageCredentialTest, DefaultHostCorrect)
  {
    EXPECT_EQ(
        Azure::Storage::Details::ParseConnectionString(
            "DefaultEndpointsProtocol=https;AccountName=testaccount;AccountKey=testkey")
            .BlobServiceUri.GetHost(),
        "testaccount.blob.core.windows.net");

    EXPECT_EQ(
        Azure::Storage::Details::ParseConnectionString(
            "DefaultEndpointsProtocol=https;AccountName=targettest;AccountKey=testkey;"
            "EndpointSuffix=core.windows.net")
            .BlobServiceUri.GetHost(),
        "testaccount.blob.core.windows.net");
  }

}}} // namespace Azure::Storage::Test
