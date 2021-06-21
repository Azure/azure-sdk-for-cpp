// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include <gtest/gtest.h>

#include <azure/perf/random_stream.hpp>

TEST(circular_stream, basic)
{
  size_t const totalSize = 1024 * 1024 * 3; // should give 5 loops
  size_t const chunk = 1024 * 1024;
  auto r_stream = Azure::Perf::RandomStream::Create(totalSize);
  uint8_t buffer[chunk];
  uint8_t buffer2[chunk];

  // 1st read
  auto count = r_stream->Read(buffer, chunk, Azure::Core::Context::ApplicationContext);
  EXPECT_EQ(count, chunk);

  // 2nd read
  count = r_stream->Read(buffer2, chunk, Azure::Core::Context::ApplicationContext);
  EXPECT_EQ(count, chunk);
  for (size_t i = 0; i != chunk; i++)
  {
    EXPECT_EQ(buffer[i], buffer2[i]);
  }

  // 3nd read
  count = r_stream->Read(buffer, chunk, Azure::Core::Context::ApplicationContext);
  EXPECT_EQ(count, chunk);
  for (size_t i = 0; i != chunk; i++)
  {
    EXPECT_EQ(buffer[i], buffer2[i]);
  }

  // 4nd read
  count = r_stream->Read(buffer, chunk, Azure::Core::Context::ApplicationContext);
  EXPECT_EQ(count, size_t(0));
  // should not change buffer
  for (size_t i = 0; i != chunk; i++)
  {
    EXPECT_EQ(buffer[i], buffer2[i]);
  }
}
