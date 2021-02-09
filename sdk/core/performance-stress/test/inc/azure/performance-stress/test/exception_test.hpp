// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

/**
 * @file
 * @brief Measures the overhead of creating, throwing, and catching an exception (compared to
 * NoOpTest).
 *
 */

#pragma once

#include <azure/performance_framework.hpp>

#include <memory>

namespace Azure { namespace PerformanceStress { namespace Test {

  /**
   * @brief Measures the overhead of creating, throwing, and catching an exception (compared to
   * NoOpTest).
   *
   */
  class ExceptionTest : public Azure::PerformanceStress::PerformanceTest {
  public:
    /**
     * @brief Construct a new Exception test.
     *
     * @param options The test options.
     */
    ExceptionTest(Azure::PerformanceStress::TestOptions options) : PerformanceTest(options) {}

    /**
     * @brief Test throwing and catching.
     *
     */
    void Run(Azure::Core::Context const&) override
    {
      try
      {
        throw std::runtime_error("Some error");
      }
      catch (std::runtime_error const&)
      {
        // just ignore
      }
    }

    /**
     * @brief Get the static Test Metadata for the test.
     *
     * @return Azure::PerformanceStress::TestMetadata describing the test.
     */
    static Azure::PerformanceStress::TestMetadata GetTestMetadata()
    {
      return {
          "exception",
          "Measure how the impact of catching a runtime exception.",
          [](Azure::PerformanceStress::TestOptions options) {
            return std::make_unique<Azure::PerformanceStress::Test::ExceptionTest>(options);
          }};
    }
  };

}}} // namespace Azure::PerformanceStress::Test
