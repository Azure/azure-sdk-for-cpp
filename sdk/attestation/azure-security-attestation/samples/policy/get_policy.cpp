// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

/**
 * @brief This sample demonstrates using the Attestation Service SDK for C++ to retrieve attestation
 * policies from an AAD mode attestation service instance.
 *
 * @remark The following environment variables must be set before running the sample.
 * - ATTESTATION_AAD_URL:  Points to an Attestation Service Instance in AAD mode.
 * operations.
 *
 */

#include <azure/attestation.hpp>
#include <azure/identity.hpp>

#include <chrono>
#include <get_env.hpp>
#include <iomanip>
#include <iostream>
#include <thread>
#include <vector>

// cspell:: words mrsigner mrenclave mitm
using namespace Azure::Security::Attestation;
using namespace Azure::Security::Attestation::Models;
using namespace std::chrono_literals;
using namespace Azure::Core;

int main()
{
  try
  {
    // create an administration client
    auto const credential = std::make_shared<Azure::Identity::AzureCliCredential>();

    AttestationAdministrationClient adminClient(AttestationAdministrationClient::Create(
        GetEnvHelper::GetEnv("ATTESTATION_AAD_URL"), credential));

    // Retrieve the SGX Attestation Policy from this attestation service instance.
    Azure::Response<AttestationToken<std::string>> const sgxPolicy
        = adminClient.GetAttestationPolicy(AttestationType::SgxEnclave);
    std::cout << "SGX Attestation Policy is: " << sgxPolicy.Value.Body << std::endl;
  }
  catch (Azure::Core::Credentials::AuthenticationException const& e)
  {
    std::cout << "Authentication Exception happened:" << std::endl << e.what() << std::endl;
    return 1;
  }
  catch (Azure::Core::RequestFailedException const& e)
  {
    std::cout << "Request Failed Exception happened:" << std::endl << e.what() << std::endl;
    if (e.RawResponse)
    {
      std::cout << "Error Code: " << e.ErrorCode << std::endl;
      std::cout << "Error Message: " << e.Message << std::endl;
    }
    return 1;
  }
  return 0;
}
