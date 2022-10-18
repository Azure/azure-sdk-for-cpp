// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include <azure/perf.hpp>

#include "azure/keyvault/keys/test/get_key_test.hpp"

int main(int argc, char** argv)
{
  std::cout << "AZURE_IDENTITY VERSION " << VCPKG_IDENTITY_VERSION << std::endl;
  std::cout << "AZURE_SECURITY_KEYVAULT_KEYS_CPP VERSION " << VCPKG_KEYS_VERSION << std::endl;

  // Create the test list
  std::vector<Azure::Perf::TestMetadata> tests{
      Azure::Security::KeyVault::Keys::Test::GetKey::GetTestMetadata()};

  Azure::Perf::Program::Run(Azure::Core::Context::ApplicationContext, tests, argc, argv);

  return 0;
}
