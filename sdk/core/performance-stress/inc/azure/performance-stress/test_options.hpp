// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#pragma once

#include <azure/core/context.hpp>

#include <string>
#include <vector>

namespace Azure { namespace PerformanceStress {
  struct TestOption
  {
    std::string Name;
    std::vector<std::string> Activators;
    std::string DisplayMessage;
    uint16_t expectedArgs;
  };
}} // namespace Azure::PerformanceStress
