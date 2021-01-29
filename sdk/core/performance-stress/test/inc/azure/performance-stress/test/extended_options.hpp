// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#pragma once

#include <azure/performance-stress/options.hpp>
#include <azure/performance-stress/test.hpp>

#include <iostream>

namespace Azure { namespace PerformanceStress { namespace Test {
  struct ExtendedOptions : public Azure::PerformanceStress::Options
  {
    int extraOption = 1;
  };

  class ExtendedOptionsTest : public Azure::PerformanceStress::PerformanceTest {
  public:
    ExtendedOptionsTest(Azure::PerformanceStress::Options options) : PerformanceTest(options) {}

    void Run(Azure::Core::Context const& ctx) override
    {
      (void)ctx;
      std::cout << "Test No Op" << std::endl;
    }
  };

}}} // namespace Azure::PerformanceStress::Test
