// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#pragma once

#include <azure/performance-stress/options.hpp>
#include <azure/performance-stress/test.hpp>

#include <iostream>

namespace Azure { namespace PerformanceStress { namespace Test {
  class NoOp : public Azure::PerformanceStress::PerformanceTest {
  public:
    NoOp(Azure::PerformanceStress::TestOptions options) : PerformanceTest(options) {}

    void Run(Azure::Core::Context const& ctx) override { (void)ctx; }
  };

}}} // namespace Azure::PerformanceStress::Test
