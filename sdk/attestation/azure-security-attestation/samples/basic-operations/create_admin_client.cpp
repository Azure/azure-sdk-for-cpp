// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

/**
 * @brief This sample shows how to instantiate an attestation administration client object using
 * the Attestation SDK client for C++.
 *
 * @remark The following environment variables must be set before running the sample.
 * - ATTESTATION_AAD_URL:  Points to an Attestation Service Instance in AAD mode.
 * - ATTESTATION_ISOLATED_URL:  Points to an Attestation Service Instance in Isolated mode.
 * operations.
 * - AZURE_TENANT_ID:     Tenant ID for the Azure account.
 * - AZURE_CLIENT_ID:     The Client ID to authenticate the request.
 * - AZURE_CLIENT_SECRET: The client secret.
 *
 * Note that the administration client MUST be authenticated.
 *
 */

#include <get_env.hpp>

#include <azure/attestation.hpp>
#include <azure/identity.hpp>
#include <chrono>
#include <iostream>
#include <memory>
#include <thread>

using namespace Azure::Security::Attestation;
using namespace Azure::Security::Attestation::Models;
using namespace std::chrono_literals;

int main()
{
  try
  {
    // create client
    auto const credential = std::make_shared<Azure::Identity::ClientSecretCredential>(
        GetEnvHelper::GetEnv("AZURE_TENANT_ID"),
        GetEnvHelper::GetEnv("AZURE_CLIENT_ID"),
        GetEnvHelper::GetEnv("AZURE_CLIENT_SECRET"));
    AttestationAdministrationClient const adminClient(AttestationAdministrationClient::Create(
        GetEnvHelper::GetEnv("ATTESTATION_AAD_URL"), credential));

    std::cout << "Admin client is Communicating with " << adminClient.Endpoint() << std::endl;
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
