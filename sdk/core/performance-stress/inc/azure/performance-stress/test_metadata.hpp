// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

/**
 * @file
 * @brief Define the metadata of a performance test.
 *
 */

#pragma once

#include "azure/performance-stress/test.hpp"
#include "azure/performance-stress/test_options.hpp"

#include <functional>
#include <memory>
#include <string>

namespace Azure { namespace PerformanceStress {
  /**
   * @brief Define the metadata of a test that can be run by the performance framework.
   *
   */
  struct TestMetadata
  {
    /**
     * @brief The name of the test.
     *
     */
    std::string Name;
    /**
     * @brief Describe the goal or intention of the test.
     *
     */
    std::string Description;

    /**
     * @brief The callback function which produces the performance test.
     *
     */
    std::function<std::unique_ptr<Azure::PerformanceStress::PerformanceTest>(
        Azure::PerformanceStress::TestOptions)>
        Factory;
  };
}} // namespace Azure::PerformanceStress
