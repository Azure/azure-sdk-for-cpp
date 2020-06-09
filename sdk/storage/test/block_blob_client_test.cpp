// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include "test_base.hpp"
#include "blobs/blob.hpp"

TEST(BlockBlobClientTest, StageBlockTest)
{
  EXPECT_EQ("", std::string(Azure::Storage::Test::TestUtility::k_ACCOUNT_SAS));
}
