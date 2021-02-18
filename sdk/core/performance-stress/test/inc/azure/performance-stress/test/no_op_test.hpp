// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

/**
 * @file
 * @brief Define an empty test.
 *
 */

#pragma once

#include <azure/performance_framework.hpp>

#include <memory>

namespace Azure { namespace PerformanceStress { namespace Test {

  /**
   * @brief The no op test is an empty test used to measure the performance framework alone.
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
    void Run(Azure::Core::Context const&) override {}

    /**
     * @brief Get the static Test Metadata for the test.
     *
     * @return Azure::PerformanceStress::TestMetadata describing the test.
     */
    static Azure::PerformanceStress::TestMetadata GetTestMetadata()
    {
      return {
          "NoOp",
          "Simplest test to measure the performance framework speed.",
          [](Azure::PerformanceStress::TestOptions options) {
            return std::make_unique<Azure::PerformanceStress::Test::NoOp>(options);
          }};
    }
  };

}}} // namespace Azure::PerformanceStress::Test
