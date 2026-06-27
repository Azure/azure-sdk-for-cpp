// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include <azure/perf/process_stats.hpp>

#include <chrono>
#include <thread>

#include <gtest/gtest.h>

using Azure::Perf::ProcessStatsSampler;

TEST(process_stats, start_stop)
{
  ProcessStatsSampler s(std::chrono::milliseconds(50));
  s.Start();
  // Burn a little CPU so there is something to sample.
  auto deadline = std::chrono::steady_clock::now() + std::chrono::milliseconds(250);
  volatile uint64_t x = 0;
  while (std::chrono::steady_clock::now() < deadline)
  {
    for (int i = 0; i < 10000; ++i)
    {
      x += i;
    }
  }
  s.Stop();
  auto avg = s.Average();
  // CPU and memory must be non-negative; we cannot assert tighter bounds in CI.
  EXPECT_GE(avg.CpuPercent, 0.0);
  // MemoryBytes is unsigned; just sanity-check accessor.
  (void)avg.MemoryBytes;
}

TEST(process_stats, reset_clears)
{
  ProcessStatsSampler s(std::chrono::milliseconds(50));
  s.Start();
  std::this_thread::sleep_for(std::chrono::milliseconds(150));
  s.Stop();
  s.Reset();
  auto avg = s.Average();
  EXPECT_DOUBLE_EQ(avg.CpuPercent, 0.0);
  EXPECT_EQ(avg.MemoryBytes, 0u);
}
