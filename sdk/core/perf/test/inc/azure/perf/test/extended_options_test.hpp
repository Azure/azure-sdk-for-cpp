// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

/**
 * @file
 * @brief An example of a performance test that defines a test option.
 *
 */

#pragma once

#include <azure/perf.hpp>

#include <memory>
#include <vector>

namespace Azure { namespace Perf { namespace Test {

  /**
   * @brief A performance test that defines a test option.
   *
   */
  class ExtendedOptionsTest : public Azure::Perf::PerfTest {
  public:
    /**
     * @brief Construct a new Extended Options Test object.
     *
     * @param options The command-line parsed options.
     */
    ExtendedOptionsTest(Azure::Perf::TestOptions options) : PerfTest(options) {}

    /**
     * @brief The test definition
     *
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
    std::vector<Azure::Perf::TestOption> GetTestOptions() override
    {
      return {{"extraOption", {"-e"}, "Example for extended option for test.", 1}};
    }

    /**
     * @brief Get the static Test Metadata for the test.
     *
     * @return Azure::Perf::TestMetadata describing the test.
     */
    static Azure::Perf::TestMetadata GetTestMetadata()
    {
      return {
          "extendedOptions",
          "Demostrate how to include a test option to a test and measures how expensive is to do "
          "it.",
          [](Azure::Perf::TestOptions options) {
            return std::make_unique<Azure::Perf::Test::ExtendedOptionsTest>(options);
          }};
    }
  };

}}} // namespace Azure::Perf::Test