//  Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include <azure/storage/common/storage_credential.hpp>

#include "test_base.hpp"

namespace Azure { namespace Storage { namespace Test {

  TEST(StorageCredentialTest, DefaultHostCorrect)
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
  }

}}} // namespace Azure::Storage::Test
