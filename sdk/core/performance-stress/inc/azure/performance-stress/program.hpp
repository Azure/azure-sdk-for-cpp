// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#pragma once

#include "azure/performance-stress/argagg.hpp"
#include "azure/performance-stress/test.hpp"

#include <functional>
#include <iostream>
#include <map>
#include <memory>
#include <string>

namespace Azure { namespace PerformanceStress {
  class Program {
  private:
    class ArgParser {
    public:
      static argagg::parser_results Parse(
          int argc,
          char** argv,
          std::vector<Azure::PerformanceStress::TestOption> const& testOptions);

      static Azure::PerformanceStress::GlobalTestOptions Parse(
          argagg::parser_results const& parsedArgs);
    };

  public:
    static void Run(
        Azure::Core::Context const& context,
        std::map<
            std::string,
            std::function<std::unique_ptr<Azure::PerformanceStress::PerformanceTest>(
                Azure::PerformanceStress::TestOptions)>> const& tests,
        int argc,
        char** argv);
  };
}} // namespace Azure::PerformanceStress
