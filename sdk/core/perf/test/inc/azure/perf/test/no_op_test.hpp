// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

/**
 * @file
 * @brief Define an empty test.
 *
 */

#pragma once

#include <azure/perf.hpp>

#include <memory>

namespace Azure { namespace Perf { namespace Test {

  /**
   * @brief The no op test is an empty test used to measure the performance framework alone.
   *
   */
  class NoOp : public Azure::Perf::PerfTest {
  public:
    /**
     * @brief Construct a new No Op test.
     *
     * @param options The test options.
     */
    NoOp(Azure::Perf::TestOptions options) : PerfTest(options) {}

    /**
     * @brief Define an empty test.
     *
     */
    void Run(Azure::Core::Context const&) override
    { // Get the option or a default value of 0
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
          "NoOp",
          "Simplest test to measure the performance framework speed.",
          [](Azure::Perf::TestOptions options) {
            return std::make_unique<Azure::Perf::Test::NoOp>(options);
          }};
    }
  };

}}} // namespace Azure::Perf::Test