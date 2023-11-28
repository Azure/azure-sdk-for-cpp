// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

/**
 * @brief This sample provides the code implementation to use the Key Vault Certificates SDK client
 * for C++ to create, get, update, delete and purge a certificate.
 *
 * @remark The following environment variables must be set before running the sample.
 * - ATTESTATION_AAD_URL:  Points to an Attestation Service Instance in AAD mode.
 * - ATTESTATION_ISOLATED_URL:  Points to an Attestation Service Instance in Isolated mode.
 * - LOCATION_SHORT_NAME:  Specifies the short name of an Azure region to use for shared mode
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
    std::string const endpoint(GetEnvHelper::GetEnv("ATTESTATION_AAD_URL"));

    // Attestation tokens returned by the service should be issued by the
    // attestation service instance. Update the token validation logic to ensure that
    // the right instance issued the token we received (this protects against a MITM responding
    // with a token issued by a different attestation service instance).
    AttestationAdministrationClientOptions clientOptions;
    clientOptions.TokenValidationOptions.ExpectedIssuer = endpoint;
    clientOptions.TokenValidationOptions.ValidateIssuer = true;

    // Ten seconds of clock drift are allowed between this machine and the attestation service.
    clientOptions.TokenValidationOptions.TimeValidationSlack = 10s;

    // create client
    auto const credential = std::make_shared<Azure::Identity::DefaultAzureCredential>();
    AttestationAdministrationClient const adminClient(
        AttestationAdministrationClient::Create(endpoint, credential, clientOptions));

    Azure::Response<AttestationToken<PolicyResult>> const resetResult
        = adminClient.ResetAttestationPolicy(AttestationType::SgxEnclave);

    if (resetResult.Value.Body.PolicyResolution == PolicyModification::Removed)
    {
      std::cout << "Attestation policy was reset." << std::endl;
    }
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
