// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

/**
 * @file
 * @brief Run-summary helpers: `--results-file` writer and `#StartJobStatistics` printer.
 *
 */

#pragma once

#include "azure/perf/latency_stats.hpp"

#include <cstdint>
#include <string>
#include <vector>

namespace Azure { namespace Perf {

  /**
   * @brief A consolidated run summary used by the framework. Fields mirror the data
   * already printed in the .NET reference framework's results block.
   *
   */
  struct RunSummary
  {
    std::string TestName;
    int Parallel = 1;
    int DurationSeconds = 0;
    int Warmup = 0;
    int Iterations = 1;
    uint64_t TotalOperations = 0;
    double WeightedAverageSeconds = 0;
    double OperationsPerSecond = 0;
    double SecondsPerOperation = 0;
    LatencyCollector::Summary Latency;
    std::vector<std::pair<std::string, LatencyCollector::Summary>> LatencyByCallType;
  };

  /**
   * @brief A single per-operation result, matching the .NET
   * `Azure.Test.Perf.OperationResult { Time, Size }` schema.
   *
   * `Time` is the operation latency in milliseconds; `Size` is the operation size in
   * bytes (or -1 if the test does not have a meaningful size).
   *
   */
  struct OperationResult
  {
    double Time = 0;
    int64_t Size = -1;
  };

  /**
   * @brief Write per-operation results to `path` as a JSON array of
   * `OperationResult { Time, Size }` objects, matching the .NET `--results-file` output
   * shape.
   *
   * @param path Destination file.
   * @param results The per-operation samples to write.
   */
  void WriteResultsFile(std::string const& path, std::vector<OperationResult> const& results);

  /**
   * @brief Print the `#StartJobStatistics`/`#EndJobStatistics` JSON block consumed by the
   * perf-automation tool.
   *
   * @details The payload matches the .NET reference framework's `BenchmarkOutput`
   * envelope:
   * ```
   * { "Metadata": [
   *     {"Source","Name","ShortDescription","LongDescription","Format"}
   *   ],
   *   "Measurements": [
   *     {"Timestamp","Name","Value"}
   *   ]
   * }
   * ```
   *
   * @param summary The run summary to serialize.
   */
  void PrintJobStatistics(RunSummary const& summary);

}} // namespace Azure::Perf
