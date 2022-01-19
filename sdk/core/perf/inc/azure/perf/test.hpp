// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

/**
 * @file
 * @brief Define a Performance test with options.
 *
 */

#pragma once

#include "azure/perf/base_test.hpp"
#include "azure/perf/dynamic_test_options.hpp"

#include <memory>

namespace Azure { namespace Perf {
  /**
   * @brief Define a performance test with options.
   *
   */
  class PerfTest : public Azure::Perf::BaseTest {

  public:
    /**
     * @brief Construct a new Performance Test.
     *
     * @param options The command-line parsed options.
     */
    PerfTest(Azure::Perf::TestOptions options) : BaseTest(options) {}

    /**
     * @brief Destroy the Performance Test object.
     *
     */
    virtual ~PerfTest() {}
  };
}} // namespace Azure::Perf
