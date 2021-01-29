// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#pragma once

#include <azure/core/context.hpp>

#include "azure/performance-stress/test_options.hpp"

#include <string>
#include <vector>

namespace Azure { namespace PerformanceStress {

  // contract for a test
  struct BaseTest
  {
    // Before starting test
    virtual void GlobalSetup(){};
    // Before running test each time
    virtual void Setup(){};

    virtual std::vector<Azure::PerformanceStress::TestOption> GetTestOptions()
    {
      return std::vector<Azure::PerformanceStress::TestOption>();
    };

    virtual void Run(Azure::Core::Context const& cancellationToken) = 0;
    // async not supported on Azure SDK for C++
    // virtual void RunAsync(Azure::Core::Context cancellationToken) = 0;

    // Run after test end
    virtual void Cleanup(){};
    // Run before application end
    virtual void GlobalCleanup(){};
  };
}} // namespace Azure::PerformanceStress
