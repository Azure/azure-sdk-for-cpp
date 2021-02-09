// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

/**
 * @file
 * @brief Measures the overhead of creating, throwing, and catching an exception (compared to
 * NoOpTest).
 *
 */

#pragma once

#include <azure/performance-stress/options.hpp>
#include <azure/performance-stress/test.hpp>

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
  };

}}} // namespace Azure::PerformanceStress::Test
