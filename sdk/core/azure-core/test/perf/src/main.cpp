// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include <azure/perf.hpp>

#include "azure/core/test/perf/nullable.hpp"

#include <vector>

int main(int argc, char** argv)
{

  // Create the test list
  std::vector<Azure::Perf::TestMetadata> tests{
      Azure::Core::Test::Performance::NullableTest::GetTestMetadata()};

  Azure::Perf::Program::Run(Azure::Core::GetApplicationContext(), tests, argc, argv);

  return 0;
}
