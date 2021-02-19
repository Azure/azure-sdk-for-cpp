// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include <azure/perf.hpp>

#include "azure/identity/test/perf/secret_credential.hpp"

int main(int argc, char** argv)
{

  // Create the test list
  std::vector<Azure::Perf::TestMetadata> tests{
      Azure::Identity::Test::Performance::SecretCredentialTest::GetTestMetadata()};

  Azure::Perf::Program::Run(Azure::Core::GetApplicationContext(), tests, argc, argv);

  return 0;
}
