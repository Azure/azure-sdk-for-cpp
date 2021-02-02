// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

/**
 * @file
 * @brief Define a Performance test with options.
 *
 */

#pragma once

#include "azure/performance-stress/base_test.hpp"
#include "azure/performance-stress/dynamic_test_options.hpp"
#include "azure/performance-stress/options.hpp"

#include <memory>

namespace Azure { namespace PerformanceStress {

  /**
   * @brief Define a performance test with options.
   *
   */
  class PerformanceTest : public Azure::PerformanceStress::BaseTest {
  protected:
    Azure::PerformanceStress::TestOptions m_options;

  public:
    /**
     * @brief Construct a new Performance Test.
     *
     * @param options The command-line parsed options.
     */
    PerformanceTest(Azure::PerformanceStress::TestOptions options) : m_options(options) {}

    /**
     * @brief Destroy the Performance Test object.
     *
     */
    virtual ~PerformanceTest() {}
  };
}} // namespace Azure::PerformanceStress
