// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include "azure/identity/test/client_certificate_credential_test.hpp"
#include "azure/identity/test/environment_credential_test.hpp"
#include "azure/identity/test/secret_credential_test.hpp"

#include <azure/perf.hpp>

int main(int argc, char** argv)
{

  // Create the test list
  std::vector<Azure::Perf::TestMetadata> tests{
      Azure::Identity::Test::ClientCertificateCredentialTest::GetTestMetadata(),
      Azure::Identity::Test::EnvironmentCredentialTest::GetTestMetadata(),
      Azure::Identity::Test::SecretCredentialTest::GetTestMetadata()};

  Azure::Perf::Program::Run(Azure::Core::Context{}, tests, argc, argv);

  return 0;
}
