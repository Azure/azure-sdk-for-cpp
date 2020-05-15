// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#pragma once

#include "test_base.h"
#include "blobs/blob.hpp"

using namespace Azure::Storage;

TEST(BlockBlobClientTest, StageBlockTest)
{
  EXPECT_EQ(std::string(""), ::Test::TestUtility::k_ACCOUNT_SAS);
};
