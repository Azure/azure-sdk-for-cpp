//  Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

/**
 * @file
 * @brief Test the Nullable component performance.
 *
 */

#pragma once

#include <azure/perf.hpp>

#include <memory>

namespace Azure { namespace Core { namespace Test {

  /**
   * @brief Measure the Nullable object performance.
   */
  class NullableTest : public Azure::Perf::PerfTest {
  public:
    /**
     * @brief Construct a new Nullable test.
     *
     * @param options The test options.
     */
    NullableTest(Azure::Perf::TestOptions options) : PerfTest(options) {}

    /**
     * @brief Use NUllable to assing and read.
     *
     */
    void Run(Azure::Core::Context const&) override
    {
      Azure::Nullable<uint64_t> n;
      if (!n)
      {
        n = 1;
      }
      if (n)
      {
        n = 0;
      }
      auto v = n.Value();
      if (n)
      {
        n.Reset();
      }
      if (!n)
      {
        n = v;
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
          "NullableTest",
          "Measures the overhead of using nullable objects",
          [](Azure::Perf::TestOptions options) {
            return std::make_unique<Azure::Core::Test::NullableTest>(options);
          }};
    }
  };

}}} // namespace Azure::Core::Test
