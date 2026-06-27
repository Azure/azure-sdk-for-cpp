// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include <azure/perf/latency_stats.hpp>

#include <chrono>
#include <thread>
#include <vector>

#include <gtest/gtest.h>

using namespace std::chrono;
using Azure::Perf::LatencyCollector;

TEST(latency_stats, percentiles)
{
  LatencyCollector c;
  // Insert 1..100 ms; percentiles should land near the respective ranks.
  for (int i = 1; i <= 100; ++i)
  {
    c.Record(milliseconds(i));
  }
  auto s = c.Summarize();
  EXPECT_EQ(s.Count, 100u);
  EXPECT_NEAR(s.P50Ms, 50.0, 1.5);
  EXPECT_NEAR(s.P75Ms, 75.0, 1.5);
  EXPECT_NEAR(s.P90Ms, 90.0, 1.5);
  EXPECT_NEAR(s.P99Ms, 99.0, 1.5);
  // p99.9/p99.99/p99.999 on a 100-sample input all collapse near the top of the range.
  EXPECT_GE(s.P999Ms, 99.0);
  EXPECT_GE(s.P9999Ms, 99.0);
  EXPECT_GE(s.P99999Ms, 99.0);
  EXPECT_LE(s.P999Ms, 100.0);
  EXPECT_LE(s.P9999Ms, 100.0);
  EXPECT_LE(s.P99999Ms, 100.0);
  EXPECT_NEAR(s.P100Ms, 100.0, 0.001);
  EXPECT_NEAR(s.MeanMs, 50.5, 0.001);
}

TEST(latency_stats, by_call_type)
{
  LatencyCollector c;
  c.Record("A", milliseconds(10));
  c.Record("A", milliseconds(30));
  c.Record("B", milliseconds(50));
  auto byType = c.SummarizeByCallType();
  ASSERT_EQ(byType.size(), 2u);
  EXPECT_EQ(byType[0].first, "A");
  EXPECT_EQ(byType[0].second.Count, 2u);
  EXPECT_EQ(byType[1].first, "B");
  EXPECT_EQ(byType[1].second.Count, 1u);
}

TEST(latency_stats, reset)
{
  LatencyCollector c;
  c.Record(milliseconds(5));
  c.Reset();
  EXPECT_EQ(c.Summarize().Count, 0u);
}

TEST(latency_stats, concurrent_record)
{
  LatencyCollector c;
  constexpr int N = 8;
  constexpr int Per = 1000;
  std::vector<std::thread> threads;
  for (int t = 0; t < N; ++t)
  {
    threads.emplace_back([&c]() {
      for (int i = 0; i < Per; ++i)
      {
        c.Record(microseconds(100));
      }
    });
  }
  for (auto& th : threads)
  {
    th.join();
  }
  EXPECT_EQ(c.Summarize().Count, static_cast<uint64_t>(N * Per));
}
