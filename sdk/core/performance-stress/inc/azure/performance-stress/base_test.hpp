// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#pragma once

#include <azure/core/context.hpp>

namespace Azure { namespace PerformanceStress {
  // contract for a test
  struct BaseTest
  {
    // Before starting test
    virtual void GlobalSetup(){};
    // Before running test each time
    virtual void Setup(){};
    virtual void Run(Azure::Core::Context const& cancellationToken) = 0;
    // async not supported on Azure SDK for C++
    // virtual void RunAsync(Azure::Core::Context cancellationToken) = 0;

    // Run after test end
    virtual void Cleanup(){};
    // Run before application end
    virtual void GlobalCleanup(){};
  };
}} // namespace Azure::PerformanceStress
