// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

/**
 * @brief This sample demonstrates creating an attestation client using the Attestation SDK client
 * for C++.
 *
 * @remark The following environment variables must be set before running the sample.
 * - ATTESTATION_AAD_URL:  Points to an Attestation Service Instance in AAD mode.
 *
 * AttestationClient instances are not always authenticated. This sample shows unauthenticated
 * access to the client.
 *
 */

#include <get_env.hpp>

#include <azure/attestation.hpp>
#include <chrono>
#include <iostream>
#include <thread>

using namespace Azure::Security::Attestation;
using namespace Azure::Security::Attestation::Models;
using namespace std::chrono_literals;

int main()
{
  try
  {
    AttestationClientOptions clientOptions;

    // Allow up to 10s of time difference between the attestation client and the attestation
    // service.
    clientOptions.TokenValidationOptions.TimeValidationSlack = 10s;

    // create client
    std::unique_ptr<AttestationClient> attestationClient(AttestationClientFactory::Create(
        GetEnvHelper::GetEnv("ATTESTATION_AAD_URL"), clientOptions));

    attestationClient->GetOpenIdMetadata();
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
