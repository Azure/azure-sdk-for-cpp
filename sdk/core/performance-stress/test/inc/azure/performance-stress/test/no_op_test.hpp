// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

/**
 * @file
 * @brief Define an empty test.
 *
 */

#pragma once

#include <azure/performance-stress/options.hpp>
#include <azure/performance-stress/test.hpp>

namespace Azure { namespace PerformanceStress { namespace Test {

  /**
   * @brief The no op test is an empty test use to measure the performance framework alone.
   *
   */
  class NoOp : public Azure::PerformanceStress::PerformanceTest {
  public:
    /**
     * @brief Construct a new No Op test.
     *
     * @param options The test options.
     */
    NoOp(Azure::PerformanceStress::TestOptions options) : PerformanceTest(options) {}

    /**
     * @brief Define an empty test.
     *
     * @param ctx The cancellation token.
     */
    void Run(Azure::Core::Context const& ctx) override { (void)ctx; }
  };

}}} // namespace Azure::PerformanceStress::Test
