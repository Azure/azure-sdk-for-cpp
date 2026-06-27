// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

/**
 * @file
 * @brief Cross-platform CPU and resident-memory sampler for the perf framework.
 *
 */

#pragma once

#include <atomic>
#include <chrono>
#include <cstdint>
#include <mutex>
#include <thread>

namespace Azure { namespace Perf {

  /**
   * @brief Periodically samples process-wide CPU% and resident memory in a background
   * thread. Snapshots the running average and the last instantaneous values.
   *
   * @remark Designed to match the always-on sampler added in the Go perf framework, so
   * the live status line and the run summary expose `CPU` (percent) and `Memory(MiB)`
   * columns across all language SDKs.
   *
   */
  class ProcessStatsSampler {
  public:
    /**
     * @brief A point-in-time snapshot of CPU usage and resident memory.
     *
     */
    struct Snapshot
    {
      /// CPU percent of all cores combined, e.g. 250.0 means 2.5 cores busy. Never negative.
      double CpuPercent = 0.0;
      /// Resident memory in bytes (Working Set / RSS).
      uint64_t MemoryBytes = 0;
    };

    /**
     * @brief Construct a sampler with a fixed sample interval.
     *
     * @param interval Time between samples. Defaults to 1 second.
     */
    explicit ProcessStatsSampler(std::chrono::milliseconds interval = std::chrono::seconds(1));

    ~ProcessStatsSampler();

    ProcessStatsSampler(ProcessStatsSampler const&) = delete;
    ProcessStatsSampler& operator=(ProcessStatsSampler const&) = delete;

    /**
     * @brief Start sampling in a background thread. Safe to call multiple times; later
     * calls are no-ops while a sampler thread is running.
     */
    void Start();

    /**
     * @brief Stop sampling. Joins the background thread. Safe to call multiple times.
     */
    void Stop();

    /**
     * @brief Get the most recent sample (CPU percent, memory bytes).
     *
     */
    Snapshot Latest() const;

    /**
     * @brief Get the average CPU% and average memory bytes across all samples taken so
     * far. CPU% is computed from cumulative CPU-seconds against wall-clock seconds; memory
     * is the arithmetic mean of all samples.
     *
     */
    Snapshot Average() const;

    /**
     * @brief Reset all accumulated samples. Useful between iterations.
     *
     */
    void Reset();

  private:
    void Run();
    static double SampleCpuSeconds();
    static uint64_t SampleResidentMemoryBytes();

    std::chrono::milliseconds m_interval;
    std::atomic<bool> m_stop{false};
    std::thread m_thread;

    mutable std::mutex m_mutex;
    // Sampling state
    bool m_haveBaseline = false;
    double m_baselineCpuSeconds = 0.0;
    std::chrono::steady_clock::time_point m_startTime;
    Snapshot m_latest;
    // Running averages
    uint64_t m_sampleCount = 0;
    double m_memoryBytesSum = 0.0;
    double m_lastCpuSeconds = 0.0;
  };

}} // namespace Azure::Perf
