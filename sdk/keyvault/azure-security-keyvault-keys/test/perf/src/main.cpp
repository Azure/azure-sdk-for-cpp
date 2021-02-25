// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include <azure/perf.hpp>

#include "azure/keyvault/keys/test/get_key.hpp"

int main(int argc, char** argv)
{

  // Create the test list
  std::vector<Azure::Perf::TestMetadata> tests{
      Azure::Security::KeyVault::Keys::Test::GetKey::GetTestMetadata()};

  Azure::Perf::Program::Run(Azure::Core::GetApplicationContext(), tests, argc, argv);

  return 0;
}
