// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

/**
 * @file
 * @brief Measures the overhead of creating, throwing, and catching an exception (compared to
 * NoOpTest).
 *
 */

#pragma once

#include <azure/perf.hpp>

#include <memory>

namespace Azure { namespace Core { namespace Test {

  /**
   * @brief Measures the overhead of creating, throwing, and catching an exception (compared to
   * NoOpTest).
   *
   */
  class ExceptionTest : public Azure::Perf::PerfTest {
  public:
    /**
     * @brief Construct a new Exception test.
     *
     * @param options The test options.
     */
    ExceptionTest(Azure::Perf::TestOptions options) : PerfTest(options) {}

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
     * @return Azure::Perf::TestMetadata describing the test.
     */
    static Azure::Perf::TestMetadata GetTestMetadata()
    {
      return {
          "exception",
          "Measure how the impact of catching a runtime exception.",
          [](Azure::Perf::TestOptions options) {
            return std::make_unique<Azure::Core::Test::ExceptionTest>(options);
          }};
    }
  };

}}} // namespace Azure::Core::Test
