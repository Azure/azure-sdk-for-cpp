// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

/**
 * @file
 * @brief Define the interface of a performance test.
 *
 */

#pragma once

#include <azure/core/context.hpp>

#include "azure/perf/test_options.hpp"

#include <string>
#include <vector>

namespace Azure { namespace Perf {

  /**
   * @brief The base interface for a performance test.
   *
   */
  struct BaseTest
  {
    /**
     * @brief Run one time at the beggining and before any test.
     *
     * @remark No matter if the parallel option is set to more than one, the global setup will run
     * only once.
     *
     */
    virtual void GlobalSetup() {}

    /**
     * @brief Run one time per each test thread.
     *
     * @remark Each test thread will run the main test in a loop after running the setup from each
     * thead.
     *
     */
    virtual void Setup() {}

    /**
     * @brief Defines the test specific options.
     *
     * @remark The performance framework will parse the test options from the command line. The test
     * can then call `m_options.GetOptionOrDefault(optionName, defaultValue)` to get the value or
     * set a default value.
     *
     * @return The array of test options supported by a performance test.
     */
    virtual std::vector<Azure::Perf::TestOption> GetTestOptions()
    {
      return std::vector<Azure::Perf::TestOption>();
    }

    /**
     * @brief Define the main test case.
     *
     * @remark The test will run over and over in a loop until the duration of the test is reached.
     *
     * @param cancellationToken A cancellation mechanism for the .
     */
    virtual void Run(Azure::Core::Context const& cancellationToken) = 0;

    /**
     * @brief Run one per test thread once that the main test loop finishes.
     *
     * @remark The clean up can be skipped by setting the option `NoCleanup`.
     */
    virtual void Cleanup() {}

    /**
     * @brief Run only once before the test application ends.
     *
     */
    virtual void GlobalCleanup() {}
  };
}} // namespace Azure::Perf
