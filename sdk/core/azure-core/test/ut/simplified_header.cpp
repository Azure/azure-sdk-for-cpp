// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

/**
 * @brief makes sure azure/core.hpp can be included.
 *
 * @remark This file will catch any issue while trying to use/include the core.hpp header
 *
 */

#include "gtest/gtest.h"
#include <azure/core.hpp>

#include <vector>

TEST(Logging, simplifiedHeader)
{
  EXPECT_NO_THROW(Azure::Core::Context c);
  EXPECT_NO_THROW(Azure::Core::DateTime::DateTime::FromSeconds(10));
  EXPECT_NO_THROW(Azure::Core::Nullable<int> n);
  EXPECT_NO_THROW(Azure::Core::Http::RawResponse r(
      1, 1, Azure::Core::Http::HttpStatusCode::Accepted, "phrase"));
  EXPECT_NO_THROW(Azure::Core::Strings::ToLower("A"));
  EXPECT_NO_THROW(Azure::Core::Uuid::CreateUuid());
  EXPECT_NO_THROW(Azure::Core::Version::VersionString());

  {
    std::vector<uint8_t> buffer(10);
    EXPECT_NO_THROW(Azure::Core::Http::MemoryBodyStream mb(buffer));
  }
  EXPECT_NO_THROW(Azure::Core::Http::TelemetryPolicy tp("", ""));
  EXPECT_NO_THROW(Azure::Core::Http::HttpPipeline pipeline(
      std::vector<std::unique_ptr<Azure::Core::Http::HttpPolicy>>(1)));
}
