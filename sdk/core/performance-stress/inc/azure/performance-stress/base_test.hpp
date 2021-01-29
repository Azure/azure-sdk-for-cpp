// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#pragma once

#include <azure/core/context.hpp>

namespace Azure { namespace PerformanceStress {
  // contract for a test
  struct BaseTest
  {
    virtual void GlobalSetupAsync(){};
    // virtual void SetupAsync(){};
    virtual void Run(Azure::Core::Context const& cancellationToken) = 0;
    // async not supported on Azure SDK for C++
    // virtual void RunAsync(Azure::Core::Context cancellationToken) = 0;
    // virtual void CleanupAsync(){};
    virtual void GlobalCleanupAsync(){};
  };
}} // namespace Azure::PerformanceStress
