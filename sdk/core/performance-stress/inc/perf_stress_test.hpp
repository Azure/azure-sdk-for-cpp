// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#pragma once

#include "perf_stress_options.hpp"
#include "perf_stress_test_base.hpp"

namespace Azure { namespace PerfStress {
  class PerfStressTest : public Azure::PerfStress::PerfStressTestBase {
  private:
    Azure::PerfStress::PerfStressOptions m_options;

  public:
    PerfStressTest(Azure::PerfStress::PerfStressOptions options) : m_options(std::move(options)) {}
  };
}} // namespace Azure::PerfStress
