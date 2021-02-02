// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

/**
 * @file
 * @brief Define the performance framework options.
 *
 */

#pragma once

#include <string>

#include "azure/core/nullable.hpp"
#include "azure/performance-stress/argagg.hpp"
#include "azure/performance-stress/dynamic_test_options.hpp"
#include "azure/performance-stress/test_options.hpp"

#include <azure/core/internal/json.hpp>

#include <iostream>
#include <vector>

namespace Azure { namespace PerformanceStress {
  /**
   * @brief Define the performance framework options.
   *
   */
  struct GlobalTestOptions
  {
    /**
     * @brief Define the duration of test in seconds
     *
     */
    int Duration = 10;

    /**
     * @brief Host to redirect HTTP requests.
     *
     */
    std::string Host;

    /**
     * @brief Allow untrusted SSL certs.
     *
     */
    bool Insecure;

    /**
     * @brief Number of iterations of main test loop.
     *
     * @remark The value of the iteration will multiple times the test duration.
     *
     */
    int Iterations = 1;

    /**
     * @brief Print job statistics.
     *
     */
    bool JobStatistics;

    /**
     * @brief Track and print per-operation latency statistics.
     *
     */
    bool Latency;

    /**
     * @brief Disables test clean up.
     *
     */
    bool NoCleanup;

    /**
     * @brief Number of operations to execute in parallel.
     *
     */
    int Parallel = 1;

    /**
     * @brief Port to redirect HTTP requests.
     *
     */
    Azure::Core::Nullable<int> Port;

    /**
     * @brief Target throughput (ops/sec).
     *
     */
    Azure::Core::Nullable<int> Rate;

    /**
     * @brief Duration of warmup in seconds.
     *
     */
    int Warmup = 5;

    /**
     * @brief Create an array of the performance framework options.
     *
     * @return An array of the options as #Azure::PerformanceStress::TestOption.
     */
    static std::vector<Azure::PerformanceStress::TestOption> GetOptionMetadata();
  };

  /**
   * @brief Define a #Azure::Core::Internal::Json::json to
   * Azure::PerformanceStress::GlobalTestOptions convertion.
   *
   * @remark The Json library consumes this implementation for parsing
   * #Azure::PerformanceStress::GlobalTestOptions to Json.
   *
   * @param j A Json reference to be written.
   * @param p A #Azure::PerformanceStress::GlobalTestOptions reference to be parsed.
   */
  void to_json(Azure::Core::Internal::Json::json& j, const GlobalTestOptions& p);
}} // namespace Azure::PerformanceStress
