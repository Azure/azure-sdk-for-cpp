// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

/**
 * @file
 * @brief Test the Uuid component performance.
 *
 */

#pragma once

#include <azure/core.hpp>
#include <azure/perf.hpp>

#include <memory>

namespace Azure { namespace Core { namespace Test {

  /**
   * @brief Measure the Uuid object performance.
   */
  class UuidTest : public Azure::Perf::PerfTest {
  public:
    /**
     * @brief Construct a new Uuid test.
     *
     * @param options The test options.
     */
    UuidTest(Azure::Perf::TestOptions options) : PerfTest(options) {}

    /**
     * @brief Use Uuid to assign and read.
     *
     */
    void Run(Azure::Core::Context const&) override
    {
      auto const total = m_options.GetMandatoryOption<int>("count");
      for (auto count = 0; count < total; count++)
      {
        Azure::Core::Uuid::CreateUuid();
      }
    }

    /**
     * @brief Define the test options for the test.
     *
     * @return The list of test options.
     */
    std::vector<Azure::Perf::TestOption> GetTestOptions() override
    {
      return {{"count", {"-c"}, "The number of uuid objects to be created.", 1, true}};
    }

    /**
     * @brief Get the static Test Metadata for the test.
     *
     * @return Azure::Perf::TestMetadata describing the test.
     */
    static Azure::Perf::TestMetadata GetTestMetadata()
    {
      return {
          "UuidTest",
          "Measures the overhead of using Uuid objects",
          [](Azure::Perf::TestOptions options) {
            return std::make_unique<Azure::Core::Test::UuidTest>(options);
          }};
    }
  };

}}} // namespace Azure::Core::Test