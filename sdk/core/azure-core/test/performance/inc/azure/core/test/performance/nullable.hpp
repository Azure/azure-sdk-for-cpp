// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

/**
 * @file
 * @brief Test the Nullable component performance.
 *
 */

#pragma once

#include <azure/performance_framework.hpp>

#include <memory>

namespace Azure { namespace Core { namespace Test { namespace Performance {

  /**
   * @brief Measure the Nullable object performance.
   *
   */
  class NullableTest : public Azure::PerformanceStress::PerformanceTest {
  public:
    /**
     * @brief Construct a new Nullable test.
     *
     * @param options The test options.
     */
    NullableTest(Azure::PerformanceStress::TestOptions options) : PerformanceTest(options) {}

    /**
     * @brief Use NUllable to assing and read.
     *
     * @param ctx The cancellation token.
     */
    void Run(Azure::Core::Context const&) override
    {
      Azure::Core::Nullable<uint64_t> n;
      if (!n)
      {
        n = 1;
      }
      if (n)
      {
        n = 0;
      }
      auto v = n.GetValue();
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
     * @return Azure::PerformanceStress::TestMetadata describing the test.
     */
    static Azure::PerformanceStress::TestMetadata GetTestMetadata()
    {
      return {
          "NullableTest",
          "Measures the overhead of using nullable objects",
          [](Azure::PerformanceStress::TestOptions options) {
            return std::make_unique<Azure::Core::Test::Performance::NullableTest>(options);
          }};
    }
  };

}}}} // namespace Azure::Core::Test::Performance
