// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

/**
 * @brief This sample provides the code implementation to use the Key Vault Certificates SDK client
 * for C++ to create, get, update, delete and purge a certificate.
 *
 * @remark The following environment variables must be set before running the sample.
 * - ATTESTATION_AAD_URL:  Points to an Attestation Service Instance in AAD mode.
 * - ATTESTATION_ISOLATED_URL:  Points to an Attestation Service Instance in Isolated mode.
 * - LOCATION_SHORT_NAME:  Specifies the short name of an Azure region to use for shared mode
 * operations.
 * - AZURE_TENANT_ID:     Tenant ID for the Azure account.
 * - AZURE_CLIENT_ID:     The Client ID to authenticate the request.
 * - AZURE_CLIENT_SECRET: The client secret.
 *
 */

#include "get_env.hpp"

#include <azure/attestation.hpp>
#include <azure/core/base64.hpp>
#include <azure/core/cryptography/hash.hpp>
#include <azure/core/internal/cryptography/sha_hash.hpp>
#include <azure/identity.hpp>

#include <chrono>
#include <iomanip>
#include <iostream>
#include <thread>
#include <vector>

using namespace Azure::Security::Attestation;
using namespace Azure::Security::Attestation::Models;
using namespace std::chrono_literals;
using namespace Azure::Core;
using namespace Azure::Core::Cryptography::_internal;

void SampleGetPolicy()
{
  // create client
  auto credential = std::make_shared<Azure::Identity::ClientSecretCredential>(
      std::getenv("AZURE_TENANT_ID"),
      std::getenv("AZURE_CLIENT_ID"),
      std::getenv("AZURE_CLIENT_SECRET"));
  AttestationAdministrationClient adminClient(std::getenv("ATTESTATION_AAD_URL"), credential);

  // Retrieve the SGX Attestation Policy from this attestation service instance.
  Azure::Response<AttestationToken<std::string>> sgxPolicy
      = adminClient.GetAttestationPolicy(AttestationType::SgxEnclave);
  std::cout << "SGX Attestation Policy is: " << sgxPolicy.Value.Body << std::endl;
}

void SampleSetPolicy()
{
  AttestationAdministrationClientOptions clientOptions;

  // Attestation tokens returned by the service should be issued by the
  // attestation service instance. Update the token validation logic to ensure that
  // the right instance issued the token we received (this protects against a MITM responding
  // with a token issued by a different attestation service instance).
  std::string endpoint(std::getenv("ATTESTATION_AAD_URL"));
  clientOptions.TokenValidationOptions.ExpectedIssuer = endpoint;
  clientOptions.TokenValidationOptions.ValidateIssuer = true;

  // Ten seconds of clock drift are allowed between this machine and the attestation service.
  clientOptions.TokenValidationOptions.ValidationTimeSlack = 10s;

  // create client
  auto credential = std::make_shared<Azure::Identity::ClientSecretCredential>(
      std::getenv("AZURE_TENANT_ID"),
      std::getenv("AZURE_CLIENT_ID"),
      std::getenv("AZURE_CLIENT_SECRET"));
  AttestationAdministrationClient adminClient(endpoint, credential, clientOptions);

  // Retrieve the OpenId metadata from this attestation service instance.

  std::string policyToSet(R"(version= 1.0;
authorizationrules 
{
	[ type=="x-ms-sgx-is-debuggable", value==true ]&&
	[ type=="x-ms-sgx-mrsigner", value=="mrsigner1"] => permit(); 
	[ type=="x-ms-sgx-is-debuggable", value==true ]&& 
	[ type=="x-ms-sgx-mrsigner", value=="mrsigner2"] => permit(); 
};)");
  Azure::Response<AttestationToken<PolicyResult>> setResult
      = adminClient.SetAttestationPolicy(AttestationType::SgxEnclave, policyToSet);

  if (*setResult.Value.Body.PolicyResolution == PolicyModification::Updated)
  {
    std::cout << "Attestation policy was updated." << std::endl;
  }

  // To verify that the attestation service received the attestation policy, the service returns
  // the SHA256 hash of the policy token which was sent ot the service. To simplify the customer
  // experience of interacting with the SetPolicy APIs, CreateSetAttestationPolicyToken API will
  // generate the same token that would be send to the service.
  //
  // To ensure that the token which was sent from the client matches the token which was received by
  // the attestation service, the customer can call CreateSetAttestationPolicyToken and then
  // generate the SHA256 of that token and compare it with the value returned by the service - the
  // two hash values should be identical.
  auto setPolicyToken = adminClient.CreateSetAttestationPolicyToken(policyToSet);
  Sha256Hash shaHasher;
  std::vector<uint8_t> policyTokenHash = shaHasher.Final(
      reinterpret_cast<uint8_t const*>(setPolicyToken.RawToken.data()),
      setPolicyToken.RawToken.size());
  std::cout << "Expected token hash: " << Convert::Base64Encode(policyTokenHash) << std::endl;
  std::cout << "Actual token hash:   "
            << Convert::Base64Encode(*setResult.Value.Body.PolicyTokenHash) << std::endl;
}

int main()
{
  try
  {
    SampleGetPolicy();
    SampleSetPolicy();
  }
  catch (Azure::Core::Credentials::AuthenticationException const& e)
  {
    std::cout << "Authentication Exception happened:" << std::endl << e.what() << std::endl;
    return 1;
  }
  catch (Azure::Core::RequestFailedException const& e)
  {
    std::cout << "Attestation Client Exception happened:" << std::endl << e.Message << std::endl;
    return 1;
  }
  return 0;
}
