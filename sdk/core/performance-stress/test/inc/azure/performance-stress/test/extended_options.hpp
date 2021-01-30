// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#pragma once

#include <azure/performance-stress/options.hpp>
#include <azure/performance-stress/test.hpp>
#include <azure/performance-stress/test_options.hpp>

#include <iostream>
#include <vector>

namespace Azure { namespace PerformanceStress { namespace Test {

  class ExtendedOptionsTest : public Azure::PerformanceStress::PerformanceTest {
  public:
    ExtendedOptionsTest(Azure::PerformanceStress::TestOptions options) : PerformanceTest(options) {}

    void Run(Azure::Core::Context const& ctx) override
    {
      (void)ctx;
      // int extraOpt = m_options.GetOptionOrDefault("extraOption", 0);
      // std::cout << "Extended option:" /*<< extraOpt*/ << std::endl;
    }

    std::vector<Azure::PerformanceStress::TestOption> GetTestOptions() override
    {
      return {{"extraOption", {"-e"}, "Example for extended option for test.", 1}};
    }
  };

}}} // namespace Azure::PerformanceStress::Test
