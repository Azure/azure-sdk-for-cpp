// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#pragma once

#include "azure/performance-stress/test.hpp"

#include <functional>
#include <iostream>
#include <memory>
#include <vector>

namespace Azure { namespace PerformanceStress {
  class Program {
  private:
    class ArgParser {
    public:
      static Azure::PerformanceStress::Options Parse(int argc, char** argv);
    };

  public:
    static void Run(
        std::vector<std::function<std::unique_ptr<Azure::PerformanceStress::PerformanceTest>(
            Azure::PerformanceStress::Options)>> const& tests,
        int argc,
        char** argv);
  };
}} // namespace Azure::PerformanceStress
