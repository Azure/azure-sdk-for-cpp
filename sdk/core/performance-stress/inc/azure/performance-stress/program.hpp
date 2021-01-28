// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#pragma once

#include "azure/performance-stress/test.hpp"

#include <functional>
#include <memory>
#include <vector>

namespace Azure { namespace PerformanceStress {
  class Program {
  public:
    static void Run(
        std::vector<std::function<std::unique_ptr<Azure::PerformanceStress::PerformanceTest>(
            Azure::PerformanceStress::Options)>> const& tests,
        int argc,
        char** argv)
    {
      (void)tests;
      (void)argc;
      (void)argv;

      // Parse options

      // Create Test

      // Run Test
    };
  };
}} // namespace Azure::PerformanceStress
