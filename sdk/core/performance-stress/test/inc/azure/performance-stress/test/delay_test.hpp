// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

/**
 * @file
 * @brief An example of a performance test that defines a test option.
 *
 */

#pragma once

#include <azure/performance_framework.hpp>

#include <atomic>
#include <chrono>
#include <memory>
#include <thread>
#include <vector>

namespace Azure { namespace PerformanceStress { namespace Test {

  namespace Details {
    static std::atomic_uint64_t DelayTestInstanceCount(0);
  }

  /**
   * @brief A performance test that defines a test option.
   *
   */
  class DelayTest : public Azure::PerformanceStress::PerformanceTest {
  private:
    std::chrono::milliseconds m_delay;

    template <class T, class V> inline uint64_t Pow(T base, V exp)
    {
      uint64_t result = 1;

      if (exp == 0)
      {
        return result;
      }
      for (V i = 0; i != exp; i++)
      {
        result = base * result;
      }
      return result;
    }

  public:
    /**
     * @brief Construct a new Extended Options Test object.
     *
     * @param options The command-line parsed options.
     */
    DelayTest(Azure::PerformanceStress::TestOptions options) : PerformanceTest(options)
    {
      // Increment the counter and fetch the value, then remove 1 to get the previous-increment
      // value
      auto instanceCount = Details::DelayTestInstanceCount.fetch_add(1) - 1;
      // default value if option is not parsed is 1000
      auto initialDelay = m_options.GetOptionOrDefault("InitialDelayMs", 1000);
      auto instanceGrowFactor = m_options.GetOptionOrDefault("InstanceGrowthFactor", 1);
      m_delay = std::chrono::milliseconds(initialDelay * Pow(instanceGrowFactor, instanceCount));
    }

    /**
     * @brief The test definition
     *
     * @param ctx The cancellation token.
     */
    void Run(Azure::Core::Context const&) override { std::this_thread::sleep_for(m_delay); }

    /**
     * @brief Define the test options for the test.
     *
     * @return The list of test options.
     */
    std::vector<Azure::PerformanceStress::TestOption> GetTestOptions() override
    {
      return {
          {"InitialDelayMs",
           {"--delay"},
           "Initial delay (in milliseconds). Default to 1000 (1sec)",
           1},
          {"InstanceGrowthFactor",
           {"--infactor"},
           "Instance growth factor. The delay of instance N will be (InitialDelayMS * "
           "(InstanceGrowthFactor ^ InstanceCount)). Default to 1",
           1},
          {"IterationGrowthFactor",
           {"--itfactor"},
           "Initial delay (in milliseconds). The delay of iteration N will be (InitialDelayMS * "
           "(IterationGrowthFactor ^ IterationCount)). Default to 1",
           1}};
    }

    /**
     * @brief Get the static Test Metadata for the test.
     *
     * @return Azure::PerformanceStress::TestMetadata describing the test.
     */
    static Azure::PerformanceStress::TestMetadata GetTestMetadata()
    {
      return {
          "delay",
          "The no op test with a configurable time delay for the main test loop.",
          [](Azure::PerformanceStress::TestOptions options) {
            return std::make_unique<Azure::PerformanceStress::Test::DelayTest>(options);
          }};
    }
  };

}}} // namespace Azure::PerformanceStress::Test
