// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include "azure/identity/test/secret_credential_test.hpp"

#include <azure/perf.hpp>

int main(int argc, char** argv)
{

  // Create the test list
  std::vector<Azure::Perf::TestMetadata> tests{
      Azure::Identity::Test::SecretCredentialTest::GetTestMetadata()};

  Azure::Perf::Program::Run(Azure::Core::Context::ApplicationContext, tests, argc, argv);

  return 0;
}
