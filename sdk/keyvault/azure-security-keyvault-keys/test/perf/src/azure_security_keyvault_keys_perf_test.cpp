// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include "azure/keyvault/keys/test/get_key_test.hpp"

#include <azure/perf.hpp>

int main(int argc, char** argv)
{

  // Create the test list
  std::vector<Azure::Perf::TestMetadata> tests{
      Azure::Security::KeyVault::Keys::Test::GetKey::GetTestMetadata()};

  Azure::Perf::Program::Run(Azure::Core::Context{}, tests, argc, argv);

  return 0;
}
