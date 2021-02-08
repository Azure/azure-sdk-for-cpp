// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

/**
 * @brief makes sure azure/core.hpp can be included.
 *
 * @remark This file will catch any issue while trying to use/include the core.hpp header
 *
 */

#include <azure/core.hpp>
#include <gtest/gtest.h>

#include <vector>

class DllExportTest {
  AZ_CORE_DLLEXPORT static const bool DllExportHIncluded;
};

TEST(SimplifiedHeader, core)
{
  EXPECT_NO_THROW(Azure::Core::Context c);
  EXPECT_NO_THROW(Azure::Core::DateTime(2020, 11, 03, 15, 30, 44));
  EXPECT_NO_THROW(Azure::Core::ETag e);
  EXPECT_NO_THROW(Azure::Core::Base64Decode("foo"));
  EXPECT_NO_THROW(Azure::Core::Md5 m);
  EXPECT_NO_THROW(Azure::Core::Http::RawResponse r(
      1, 1, Azure::Core::Http::HttpStatusCode::Accepted, "phrase"));
  EXPECT_NO_THROW(Azure::Core::MatchConditions mc);
  EXPECT_NO_THROW(Azure::Core::Nullable<int> n);
  EXPECT_NO_THROW(Azure::Core::Uuid::CreateUuid());
  EXPECT_NO_THROW(Azure::Core::RequestConditions rc);
  EXPECT_NO_THROW(Azure::Core::Details::Version::VersionString());

  {
    std::vector<uint8_t> buffer(10);
    EXPECT_NO_THROW(Azure::Core::Http::MemoryBodyStream mb(buffer));
  }
  EXPECT_NO_THROW(Azure::Core::Http::TelemetryPolicy tp("", ""));
  EXPECT_NO_THROW(Azure::Core::Http::HttpPipeline pipeline(
      std::vector<std::unique_ptr<Azure::Core::Http::HttpPolicy>>(1)));
}
