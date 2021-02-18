// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include <azure/performance_framework.hpp>

#include "azure/keyvault/keys/test/performance/get_key.hpp"

int main(int argc, char** argv)
{

  // Create the test list
  std::vector<Azure::PerformanceStress::TestMetadata> tests{
      Azure::Security::KeyVault::Keys::Test::Performance::GetKey::GetTestMetadata()};

  Azure::PerformanceStress::Program::Run(Azure::Core::GetApplicationContext(), tests, argc, argv);

  return 0;
}
