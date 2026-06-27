// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

/**
 * @file
 * @brief Per-operation latency collector and percentile summary.
 *
 */

#pragma once

#include <chrono>
#include <cstdint>
#include <mutex>
#include <string>
#include <vector>

namespace Azure { namespace Perf {

  /**
   * @brief Thread-safe collector of per-operation latency samples.
   *
   * @remark Records nanosecond-resolution durations from many worker threads and computes
   * percentile summaries (p50/p90/p95/p99/max) on demand. Designed to match the latency
   * reporting added in the Go perf framework so cross-language results are comparable.
   *
   */
  class LatencyCollector {
  public:
    /**
     * @brief A single latency sample, optionally tagged by call type.
     *
     */
    struct Sample
    {
      std::chrono::nanoseconds Duration{0};
      std::string CallType;
    };

    /**
     * @brief Latency summary expressed in milliseconds, matching the .NET
     * `Azure.Test.Perf` percentile distribution: 50, 75, 90, 99, 99.9, 99.99, 99.999, 100.
     *
     */
    struct Summary
    {
      uint64_t Count = 0;
      double P50Ms = 0;
      double P75Ms = 0;
      double P90Ms = 0;
      double P99Ms = 0;
      double P999Ms = 0;
      double P9999Ms = 0;
      double P99999Ms = 0;
      double P100Ms = 0;
      double MeanMs = 0;
    };

    /**
     * @brief Record a single latency sample with no call-type tag.
     *
     * @param duration The latency to record.
     */
    void Record(std::chrono::nanoseconds duration);

    /**
     * @brief Record a single latency sample tagged with a call type.
     *
     * @param callType A short label for the operation (e.g. "Upload").
     * @param duration The latency to record.
     */
    void Record(std::string const& callType, std::chrono::nanoseconds duration);

    /**
     * @brief Clear all recorded samples.
     *
     */
    void Reset();

    /**
     * @brief Compute the summary over all recorded samples.
     *
     * @return The percentile summary.
     */
    Summary Summarize() const;

    /**
     * @brief Compute summaries grouped by call type.
     *
     * @return A vector of (callType, summary) pairs, sorted by callType.
     */
    std::vector<std::pair<std::string, Summary>> SummarizeByCallType() const;

    /**
     * @brief Snapshot all recorded samples (copy).
     *
     */
    std::vector<Sample> Samples() const;

  private:
    mutable std::mutex m_mutex;
    std::vector<Sample> m_samples;
  };

}} // namespace Azure::Perf
