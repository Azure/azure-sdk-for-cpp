// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#pragma once

#include "azure/performance-stress/base_test.hpp"
#include "azure/performance-stress/options.hpp"

namespace Azure { namespace PerformanceStress {

  class PerformanceTest : public Azure::PerformanceStress::BaseTest {
  private:
    Azure::PerformanceStress::Options m_options;

  public:
    PerformanceTest(Azure::PerformanceStress::Options options) : m_options(std::move(options)) {}
  };
}} // namespace Azure::PerformanceStress
