// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

/**
 * @file
 * @brief An example of a performance test that defines a test option.
 *
 */

#pragma once

#include <azure/performance-stress/options.hpp>
#include <azure/performance-stress/test.hpp>
#include <azure/performance-stress/test_options.hpp>

#include <vector>

namespace Azure { namespace PerformanceStress { namespace Test {

  /**
   * @brief A performance test that defines a test option.
   *
   */
  class ExtendedOptionsTest : public Azure::PerformanceStress::PerformanceTest {
  public:
    /**
     * @brief Construct a new Extended Options Test object.
     *
     * @param options The command-line parsed options.
     */
    ExtendedOptionsTest(Azure::PerformanceStress::TestOptions options) : PerformanceTest(options) {}

    /**
     * @brief The test definition
     *
     * @param ctx The cancellation token.
     */
    void Run(Azure::Core::Context const&) override
    {
      // Get the option or a default value of 0
      auto myTestOption = m_options.GetOptionOrDefault("extraOption", 0);
      (void)myTestOption;
    }

    /**
     * @brief Define the test options for the test.
     *
     * @return The list of test options.
     */
    std::vector<Azure::PerformanceStress::TestOption> GetTestOptions() override
    {
      return {{"extraOption", {"-e"}, "Example for extended option for test.", 1}};
    }
  };

}}} // namespace Azure::PerformanceStress::Test
