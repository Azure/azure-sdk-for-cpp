// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

/**
 * @brief This sample shows how to instantiate an attestation administration client object using
 * the Attestation SDK client for C++.
 *
 * @remark The following environment variables must be set before running the sample.
 * - ATTESTATION_AAD_URL:  Points to an Attestation Service Instance in AAD mode.
 * - ATTESTATION_ISOLATED_URL:  Points to an Attestation Service Instance in Isolated mode.
 * operations.
 *
 * Note that the administration client MUST be authenticated.
 *
 */

#include <azure/attestation.hpp>
#include <azure/identity.hpp>

#include <chrono>
#include <get_env.hpp>
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
    auto const credential = std::make_shared<Azure::Identity::DefaultAzureCredential>();
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
