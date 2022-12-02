// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

/**
 * @file
 * @brief An example of a performance test that defines a test option.
 *
 */

#pragma once

#include <azure/perf.hpp>

#include <atomic>
#include <chrono>
#include <memory>
#include <thread>
#include <vector>

namespace Azure { namespace Perf { namespace Test {

  namespace _detail {
    static std::atomic_uint64_t DelayTestInstanceCount(0);
  }

  /**
   * @brief A performance test that defines a test option.
   *
   */
  class DelayTest : public Azure::Perf::PerfTest {
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
    DelayTest(Azure::Perf::TestOptions options) : PerfTest(options)
    {
      // Increment the counter and fetch the value, then remove 1 to get the previous-increment
      // value
      auto instanceCount = _detail::DelayTestInstanceCount.fetch_add(0);
      // default value if option is not parsed is 1000
      auto initialDelay = m_options.GetOptionOrDefault("InitialDelayMs", 1000);
      auto instanceGrowFactor = m_options.GetOptionOrDefault("InstanceGrowthFactor", 1);
      m_delay = std::chrono::milliseconds(initialDelay * Pow(instanceGrowFactor, instanceCount));
    }

    /**
     * @brief The test definition
     *
     */
    void Run(Azure::Core::Context const&) override { std::this_thread::sleep_for(m_delay); }

    /**
     * @brief Define the test options for the test.
     *
     * @return The list of test options.
     */
    std::vector<Azure::Perf::TestOption> GetTestOptions() override
    {
      return {
          {"InitialDelayMs", {"-m"}, "Initial delay (in milliseconds). Default to 1000 (1sec)", 1},
          {"InstanceGrowthFactor",
           {"-n"},
           "Instance growth factor. The delay of instance N will be (InitialDelayMS * "
           "(InstanceGrowthFactor ^ InstanceCount)). Default to 1",
           1},
          {"IterationGrowthFactor",
           {"-t"},
           "Initial delay (in milliseconds). The delay of iteration N will be (InitialDelayMS * "
           "(IterationGrowthFactor ^ IterationCount)). Default to 1",
           1}};
    }

    /**
     * @brief Get the static Test Metadata for the test.
     *
     * @return Azure::Perf::TestMetadata describing the test.
     */
    static Azure::Perf::TestMetadata GetTestMetadata()
    {
      return {
          "delay",
          "The no op test with a configurable time delay for the main test loop.",
          [](Azure::Perf::TestOptions options) {
            return std::make_unique<Azure::Perf::Test::DelayTest>(options);
          }};
    }
  };

}}} // namespace Azure::Perf::Test