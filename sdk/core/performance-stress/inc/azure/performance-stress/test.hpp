// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#pragma once

#include "azure/performance-stress/base_test.hpp"
#include "azure/performance-stress/dynamic_test_options.hpp"
#include "azure/performance-stress/options.hpp"

#include <memory>

namespace Azure { namespace PerformanceStress {

  class PerformanceTest : public Azure::PerformanceStress::BaseTest {
  protected:
    Azure::PerformanceStress::TestOptions m_options;

  public:
    PerformanceTest(Azure::PerformanceStress::TestOptions options) : m_options(options) {}
    virtual ~PerformanceTest(){}
  };
}} // namespace Azure::PerformanceStress
