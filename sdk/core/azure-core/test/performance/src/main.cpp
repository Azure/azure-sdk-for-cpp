// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include <azure/performance_framework.hpp>

#include "azure/core/test/performance/nullable.hpp"

#include <vector>

int main(int argc, char** argv)
{

  // Create the test list
  std::vector<Azure::PerformanceStress::TestMetadata> tests{
      Azure::Core::Test::Performance::NullableTest::GetTestMetadata()};

  Azure::PerformanceStress::Program::Run(Azure::Core::GetApplicationContext(), tests, argc, argv);

  return 0;
}
