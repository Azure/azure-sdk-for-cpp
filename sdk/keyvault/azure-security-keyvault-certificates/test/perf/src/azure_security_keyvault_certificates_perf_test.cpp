// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include "azure/keyvault/certificates/test/get_certificate_test.hpp"

#include <azure/perf.hpp>

int main(int argc, char** argv)
{

  // Create the test list
  std::vector<Azure::Perf::TestMetadata> tests{
      Azure::Security::KeyVault::Certificates::Test::GetCertificate::GetTestMetadata()};

  Azure::Perf::Program::Run(Azure::Core::Context{}, tests, argc, argv);

  return 0;
}
