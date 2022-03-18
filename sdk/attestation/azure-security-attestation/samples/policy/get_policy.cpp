// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

/**
 * @brief This sample demonstrates using the Attestation Service SDK for C++ to retrieve attestation
 * policies from an AAD mode attestation service instance.
 *
 * @remark The following environment variables must be set before running the sample.
 * - ATTESTATION_AAD_URL:  Points to an Attestation Service Instance in AAD mode.
 * operations.
 * - AZURE_TENANT_ID:     Tenant ID for the Azure account.
 * - AZURE_CLIENT_ID:     The Client ID to authenticate the request.
 * - AZURE_CLIENT_SECRET: The client secret.
 *
 */

#include "get_env.hpp"

#include <azure/attestation.hpp>
#include <azure/identity.hpp>

#include <chrono>
#include <iomanip>
#include <iostream>
#include <thread>
#include <vector>

// cspell:: words mrsigner mrenclave mitm
using namespace Azure::Security::Attestation;
using namespace Azure::Security::Attestation::Models;
using namespace std::chrono_literals;
using namespace Azure::Core;

std::string GetEnv(char const* env);

int main()
{
  try
  {
    // create an administration client
    auto credential = std::make_shared<Azure::Identity::ClientSecretCredential>(
        GetEnv("AZURE_TENANT_ID"),
        GetEnv("AZURE_CLIENT_ID"),
        GetEnv("AZURE_CLIENT_SECRET"));
    AttestationAdministrationClient adminClient(GetEnv("ATTESTATION_AAD_URL"), credential);

    // Retrieve attestation response validation collateral before calling into the service.
    adminClient.RetrieveResponseValidationCollateral();

    // Retrieve the SGX Attestation Policy from this attestation service instance.
    Azure::Response<AttestationToken<std::string>> sgxPolicy
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

std::string GetEnv(char const* env)
{
  auto const val = std::getenv(env);
  if (val == nullptr)
  {
    throw std::runtime_error("Could not find required environment variable: " + std::string(env));
  }
  return std::string(val);
}
